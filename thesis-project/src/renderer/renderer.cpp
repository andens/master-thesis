﻿#include "renderer.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <fstream>
#include <imgui.h>
#include <iostream>
#include <vector>
#include <vulkan-helpers/buffer.h>
#include <vulkan-helpers/command_buffer.h>
#include <vulkan-helpers/command_pool.h>
#include <vulkan-helpers/descriptor_pool.h>
#include <vulkan-helpers/descriptor_pool_builder.h>
#include <vulkan-helpers/descriptor_set.h>
#include <vulkan-helpers/descriptor_set_layout.h>
#include <vulkan-helpers/device.h>
#include <vulkan-helpers/device_builder.h>
#include <vulkan-helpers/instance.h>
#include <vulkan-helpers/instance_builder.h>
#include <vulkan-helpers/physical_device.h>
#include <vulkan-helpers/pipeline_builder.h>
#include <vulkan-helpers/pipeline_layout_builder.h>
#include <vulkan-helpers/pipeline_vertex_layout.h>
#include <vulkan-helpers/presentation_surface.h>
#include <vulkan-helpers/queue.h>
#include <vulkan-helpers/render_pass_builder.h>
#include <vulkan-helpers/swapchain.h>
#include <vulkan-helpers/vk_dispatch_tables.h>
#include "../depth-buffer/depth_buffer.h"
#include "../gbuffer/gbuffer.h"
#include "../gui-descriptor-set/gui-descriptor-set.h"
#include "../mesh/mesh.h"
#include "../obj-loader/obj-loader.h"
#include "../render-cache/render-cache.h"
#include "../render-jobs-descriptor-set/render-jobs-descriptor-set.h"

extern const uint32_t g_draw_call_count;

Renderer::Renderer(HWND hwnd, uint32_t render_width, uint32_t render_height) :
    render_area_ { render_width, render_height } {
  vk_globals_.reset(new vkgen::GlobalFunctions("vulkan-1.dll"));

  create_instance();
  create_debug_callback();
  create_surface(hwnd);
  create_device();
  create_swapchain();
  create_command_pools_and_buffers();
  depth_buffer_.reset(new graphics::DepthBuffer(device_.get(), render_area_.width, render_area_.height, VK_FORMAT_D32_SFLOAT, *blit_swapchain_cmd_buf_, *graphics_queue_));
  gbuffer_.reset(new graphics::deferred_shading::GBuffer(device_.get(), render_area_.width, render_area_.height));
  create_samplers();
  create_descriptor_sets();
  create_render_pass();
  create_framebuffer();
  create_shaders();
  create_pipeline();
  create_synchronization_primitives();
  configure_barrier_structs();
  create_vertex_buffer();
  create_indirect_buffer();
  initialize_imgui();
  create_dgc_resources();
  create_timing_resources();

  DirectX::XMStoreFloat4x4(&view_, DirectX::XMMatrixIdentity());
  DirectX::XMStoreFloat4x4(&proj_, DirectX::XMMatrixIdentity());

  render_cache_.reset(new RenderCache);
  use_render_strategy(render_strategy_);

  // Position the render jobs in a grid
  uint32_t rows = static_cast<uint32_t>(sqrtf(static_cast<float>(g_draw_call_count)));
  float spacing = 2.0f;
  float y = -0.5f * rows * spacing;
  for (uint32_t i = 0; i < g_draw_call_count; ++i) {
    update_transform(i, DirectX::XMMatrixTranslation((-0.5f * rows + static_cast<float>(i % rows)) * spacing, y, 140.0f));
    if (i % rows == rows - 1) {
      y += spacing;
    }
  }
}

Renderer::~Renderer() {
  if (device_->device()) {
    device_->vkDeviceWaitIdle();
    device_->vkDestroyQueryPool(query_pool_, nullptr);
    device_->vkUnmapMemory(dgc_push_constants_->vulkan_memory_handle());
    dgc_push_constants_->destroy(*device_);
    device_->vkUnmapMemory(dgc_pipeline_parameters_->vulkan_memory_handle());
    dgc_pipeline_parameters_->destroy(*device_);
    device_->vkDestroyIndirectCommandsLayoutNVX(indirect_commands_layout_, nullptr);
    device_->vkDestroyObjectTableNVX(object_table_, nullptr);
    device_->vkUnmapMemory(indirect_buffer_->vulkan_memory_handle());
    indirect_buffer_->destroy(*device_);
    vertex_buffer_->destroy(*device_);
    if (gui_index_buffer_) {
      gui_index_buffer_->destroy(*device_);
    }
    if (gui_vertex_buffer_) {
      gui_vertex_buffer_->destroy(*device_);
    }
    device_->vkDestroyImageView(gui_font_image_view_, nullptr);
    device_->vkDestroyImage(gui_font_image_, nullptr);
    device_->vkFreeMemory(gui_font_image_memory_, nullptr);
    device_->vkDestroySampler(gui_font_sampler_, nullptr);
    device_->vkDestroyFence(gbuffer_generation_fence_, nullptr);
    device_->vkDestroyFence(render_fence_, nullptr);
    device_->vkDestroySemaphore(gbuffer_generation_complete_, nullptr);
    device_->vkDestroySemaphore(blit_swapchain_complete_, nullptr);
    device_->vkDestroySemaphore(image_available_semaphore_, nullptr);
    device_->vkDestroyPipeline(pipeline_regular_mdi_wireframe_, nullptr);
    device_->vkDestroyPipeline(pipeline_regular_mdi_solid_, nullptr);
    device_->vkDestroyPipeline(pipeline_dgc_wireframe_, nullptr);
    device_->vkDestroyPipeline(pipeline_dgc_solid_, nullptr);
    device_->vkDestroyPipeline(gui_pipeline_, nullptr);
    device_->vkDestroyPipelineLayout(gbuffer_pipeline_layout_, nullptr);
    device_->vkDestroyPipelineLayout(gui_pipeline_layout_, nullptr);
    device_->vkDestroyShaderModule(fullscreen_triangle_vs_, nullptr);
    device_->vkDestroyShaderModule(fill_gbuffer_vs_, nullptr);
    device_->vkDestroyShaderModule(fill_gbuffer_fs_, nullptr);
    device_->vkDestroyShaderModule(gui_vs_, nullptr);
    device_->vkDestroyShaderModule(gui_fs_, nullptr);
    device_->vkDestroyFramebuffer(framebuffer_, nullptr);
    device_->vkDestroyRenderPass(gbuffer_render_pass_, nullptr);
    render_jobs_descriptor_set_->destroy(*device_);
    gui_descriptor_set_->destroy(*device_);
    descriptor_pool_->destroy(*device_);
    gbuffer_.reset();
    depth_buffer_.reset();
    stable_graphics_cmd_pool_.reset();
    transient_graphics_cmd_pool_.reset();
    swapchain_.reset();
    device_->vkDestroyDevice(nullptr);
  }

  if (instance_->instance()) {
    surface_.reset();
    instance_->vkDestroyDebugReportCallbackEXT(debug_callback_, nullptr);
    instance_->vkDestroyInstance(nullptr);
  }

  vk_globals_.reset();
}

void Renderer::render() {
  // ┌─────────────────────────────────────────────────────────────────┐
  // │  Generate G-buffer contents                                     │
  // └─────────────────────────────────────────────────────────────────┘

  device_->vkWaitForFences(1, &gbuffer_generation_fence_, VK_TRUE, UINT64_MAX);
  device_->vkResetFences(1, &gbuffer_generation_fence_);

  auto start_time = std::chrono::high_resolution_clock::now();

  VkCommandBufferBeginInfo begin_info {};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  begin_info.pNext = nullptr;
  begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  begin_info.pInheritanceInfo = nullptr;

  graphics_cmd_buf_->vkBeginCommandBuffer(&begin_info);

  graphics_cmd_buf_->vkCmdResetQueryPool(query_pool_, 0, 4);
  graphics_cmd_buf_->vkCmdWriteTimestamp(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, query_pool_, 0); // All previous commands must have executed until the end

  // Clear values for G-buffer attachments as indicated by the subpass
  // attachment load operations of the render pass.
  std::array<VkClearValue, 3> clear_values {};
  clear_values[0].color = { 0.0f, 0.58f, 1.0f, 0.0f };
  clear_values[1].color = { 0.5f, 0.5f, 0.5f, 0.0 };
  clear_values[2].depthStencil = { 0.0f, 0 };

  VkRenderPassBeginInfo render_pass_info {};
  render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  render_pass_info.pNext = nullptr;
  render_pass_info.renderPass = gbuffer_render_pass_;
  render_pass_info.framebuffer = framebuffer_;
  render_pass_info.renderArea = { {0, 0}, render_area_ };
  render_pass_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
  render_pass_info.pClearValues = clear_values.data();

  graphics_cmd_buf_->vkCmdBeginRenderPass(&render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

  VkViewport viewport {};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = static_cast<float>(render_area_.width);
  viewport.height = static_cast<float>(render_area_.height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  VkRect2D scissor = {};
  scissor.offset = { 0, 0 };
  scissor.extent = render_area_;
  
  // Dynamic state
  graphics_cmd_buf_->vkCmdSetViewport(0, 1, &viewport);
  graphics_cmd_buf_->vkCmdSetScissor(0, 1, &scissor);

  std::array<VkDescriptorSet, 1> descriptor_sets = {
    render_jobs_descriptor_set_->set().vulkan_handle()
  };
  graphics_cmd_buf_->vkCmdBindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, gbuffer_pipeline_layout_, 0, descriptor_sets.size(), descriptor_sets.data(), 0, nullptr);

  // TODO: This should be replaced with a proper descriptor set when the
  // command buffer will be static. Updating the descriptor set should then
  // be done in |use_matrices|. Don't forget to update the pipeline layout
  // as well.
  graphics_cmd_buf_->vkCmdPushConstants(gbuffer_pipeline_layout_, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(DirectX::XMFLOAT4X4), &view_);
  graphics_cmd_buf_->vkCmdPushConstants(gbuffer_pipeline_layout_, VK_SHADER_STAGE_VERTEX_BIT, sizeof(DirectX::XMFLOAT4X4), sizeof(DirectX::XMFLOAT4X4), &proj_);

  VkBuffer vertex_buf = vertex_buffer_->vulkan_buffer_handle();
  VkDeviceSize offset = 0;
  graphics_cmd_buf_->vkCmdBindVertexBuffers(0, 1, &vertex_buf, &offset);

  switch (render_strategy_) {
  case RenderStrategy::Regular: {
    RenderCache::Pipeline current_pipeline = RenderCache::Pipeline::Alpha;
    graphics_cmd_buf_->vkCmdBindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_regular_mdi_solid_);
    render_cache_->enumerate_all([this, &current_pipeline](RenderCache::JobContext const& job_context) -> void* {
      //graphics_cmd_buf_->vkCmdDraw(job_context.object_type == RenderObject::Box ? 36 : 2160, 1, job_context.object_type == RenderObject::Box ? 0 : 36, job_context.job);
      if (current_pipeline != job_context.pipeline) {
        graphics_cmd_buf_->vkCmdBindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, job_context.pipeline == RenderCache::Pipeline::Alpha ? pipeline_regular_mdi_solid_ : pipeline_regular_mdi_wireframe_);
        current_pipeline = job_context.pipeline;
      }
      graphics_cmd_buf_->vkCmdDraw(3, 1, job_context.object_type == RenderObject::Box ? 2196 : 2199, job_context.job);
      return job_context.user_data;
    });
    break;
  }
  case RenderStrategy::MDI: {
    uint32_t last_switched_job = 0;
    update_indirect_buffer([this, &last_switched_job](uint32_t job, VkPipeline pipeline) {
      if (job != 0) {
        graphics_cmd_buf_->vkCmdBindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
        graphics_cmd_buf_->vkCmdDrawIndirect(indirect_buffer_->vulkan_buffer_handle(), last_switched_job * sizeof(VkDrawIndirectCommand), job - last_switched_job, sizeof(VkDrawIndirectCommand));
        last_switched_job = job;
      }
    }, false);
    break;
  }
  case RenderStrategy::DGC: {
    // At this point in time we only generate the commands. The reason I split
    // DGC into two parts is that calling vkCmdProcessCommandsNVX with a null
    // target command buffer executes the generated commands immediately with
    // an implicit sync between generation and execution, thus preventing me
    // from timing just the generation. Execution is done in the next subpass.

    update_indirect_buffer([](uint32_t, VkPipeline) {}, true);

    // Before generating commands we must allocate space in the target command
    // buffer. This must be done in a render pass (reuse the current one with
    // its state), and executed in a primary command buffer because it is a
    // secondary buffer after all (the current one). We ignore recording this
    // first time, meaning that the next subpass executes the default recording
    // which happens to be the reservation command.
    static bool first = true;
    if (first) {
      first = false;
      // To not block waiting for queries that never happen.
      graphics_cmd_buf_->vkCmdWriteTimestamp(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, query_pool_, 2);
      graphics_cmd_buf_->vkCmdWriteTimestamp(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, query_pool_, 3);
      break;
    }

    graphics_cmd_buf_->vkCmdWriteTimestamp(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, query_pool_, 2); // All previous commands must have executed until the end

    // References to the input data for each token command.
    std::array<VkIndirectCommandsTokenNVX, 3> input_tokens {};
    input_tokens[0].tokenType = VK_INDIRECT_COMMANDS_TOKEN_TYPE_PIPELINE_NVX;
    input_tokens[0].buffer = dgc_pipeline_parameters_->vulkan_buffer_handle();
    input_tokens[0].offset = 0;
    input_tokens[1].tokenType = VK_INDIRECT_COMMANDS_TOKEN_TYPE_PUSH_CONSTANT_NVX;
    input_tokens[1].buffer = dgc_push_constants_->vulkan_buffer_handle();
    input_tokens[1].offset = 0;
    input_tokens[2].tokenType = VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_NVX;
    input_tokens[2].buffer = indirect_buffer_->vulkan_buffer_handle(); // Same as MDI
    input_tokens[2].offset = 0;

    VkCmdProcessCommandsInfoNVX commands_info {};
    commands_info.sType = VK_STRUCTURE_TYPE_CMD_PROCESS_COMMANDS_INFO_NVX;
    commands_info.pNext = nullptr;
    commands_info.objectTable = object_table_;
    commands_info.indirectCommandsLayout = indirect_commands_layout_;
    commands_info.indirectCommandsTokenCount = static_cast<uint32_t>(input_tokens.size());
    commands_info.pIndirectCommandsTokens = input_tokens.data();
    commands_info.maxSequencesCount = g_draw_call_count; // Actual count because of no count buffer
    commands_info.targetCommandBuffer = dgc_cmd_buf_->command_buffer();
    commands_info.sequencesCountBuffer = VK_NULL_HANDLE; // Don't source count from a buffer, I provide actual count in |maxSequencesCount|
    commands_info.sequencesCountOffset = 0; // Not used (no sequencesCountBuffer)
    commands_info.sequencesIndexBuffer = VK_NULL_HANDLE; // No custom sequence indices; rely on default 1, 2, ....
    commands_info.sequencesIndexOffset = 0; // Not used (no sequencesIndexBuffer)
    // Why would not a command with maxSequencesCount == 0 work? If I do that,
    // the geometry is rendered anyways. The command buffer probably is reset
    // because the object properly vanishes if I don't call the method, but it
    // really should work even when I tell it to generate no sequence at all.
    //if (current_total_draw_calls_ > 0) {
      graphics_cmd_buf_->vkCmdProcessCommandsNVX(&commands_info);
    //}

    // Synchronize the generation of commands in this pass before they are
    // consumed in the next.
    VkMemoryBarrier dgc_generation_barrier {};
    dgc_generation_barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    dgc_generation_barrier.pNext = nullptr;
    dgc_generation_barrier.srcAccessMask = VK_ACCESS_COMMAND_PROCESS_WRITE_BIT_NVX; // Write command buffer output (read is reading input buffers)
    dgc_generation_barrier.dstAccessMask = VK_ACCESS_INDIRECT_COMMAND_READ_BIT; // Output is consumed by the draw indirect pipeline stage (reading)
    graphics_cmd_buf_->vkCmdPipelineBarrier(
      VK_PIPELINE_STAGE_COMMAND_PROCESS_BIT_NVX,
      VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
      0,
      1, &dgc_generation_barrier,
      0, nullptr,
      0, nullptr
    );

    // The intention here is to not wait for anything. At the time this query
    // is reached, the barrier should have waited for the writes to finish.
    graphics_cmd_buf_->vkCmdWriteTimestamp(VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT, query_pool_, 3);

    break;
  }
  default: throw;
  }

  // If the strategy is DGC, the next subpass executes the commands generated
  // in the first one.
  graphics_cmd_buf_->vkCmdNextSubpass(VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

  if (render_strategy_ == RenderStrategy::DGC) {
    VkCommandBuffer cmd_buf = dgc_cmd_buf_->command_buffer();
    graphics_cmd_buf_->vkCmdExecuteCommands(1, &cmd_buf);
  }

  graphics_cmd_buf_->vkCmdNextSubpass(VK_SUBPASS_CONTENTS_INLINE);

  if (!measure_session_active_) {
    ImDrawData* draw_data = ImGui::GetDrawData();
    if (draw_data->TotalVtxCount != 0) {
      update_gui_vertex_data(draw_data);

      graphics_cmd_buf_->vkCmdBindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, gui_pipeline_);

      {
        VkDescriptorSet desc_set = gui_descriptor_set_->set().vulkan_handle();
        graphics_cmd_buf_->vkCmdBindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, gui_pipeline_layout_, 0, 1, &desc_set, 0, nullptr);
      }

      {
        VkBuffer vert_buf = gui_vertex_buffer_->vulkan_buffer_handle();
        VkDeviceSize vertex_offset = 0;
        graphics_cmd_buf_->vkCmdBindVertexBuffers(0, 1, &vert_buf, &vertex_offset);
        graphics_cmd_buf_->vkCmdBindIndexBuffer(gui_index_buffer_->vulkan_buffer_handle(), 0, VK_INDEX_TYPE_UINT16);
      }

      {
        VkViewport viewport {};
        viewport.x = 0;
        viewport.y = 0;
        viewport.width = ImGui::GetIO().DisplaySize.x;
        viewport.height = ImGui::GetIO().DisplaySize.y;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        graphics_cmd_buf_->vkCmdSetViewport(0, 1, &viewport);
      }

      // Scale and translation
      {
        ImGuiIO& io = ImGui::GetIO();
        float scale[2] = { 2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y };
        float translate[2] = { -1.0f, -1.0f };
        graphics_cmd_buf_->vkCmdPushConstants(gui_pipeline_layout_, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(float) * 2, scale);
        graphics_cmd_buf_->vkCmdPushConstants(gui_pipeline_layout_, VK_SHADER_STAGE_VERTEX_BIT, sizeof(float) * 2, sizeof(float) * 2, translate);
      }

      // Render the command lists
      int vertex_offset = 0;
      int index_offset = 0;
      for (int n = 0; n < draw_data->CmdListsCount; ++n) {
        ImDrawList const* cmd_list = draw_data->CmdLists[n];
        for (int cmd_num = 0; cmd_num < cmd_list->CmdBuffer.Size; ++cmd_num) {
          ImDrawCmd const* draw_cmd = &cmd_list->CmdBuffer[cmd_num];
          if (draw_cmd->UserCallback) {
            draw_cmd->UserCallback(cmd_list, draw_cmd);
          }
          else {
            VkRect2D scissor {};
            scissor.offset.x = draw_cmd->ClipRect.x > 0.0f ? static_cast<int32_t>(draw_cmd->ClipRect.x) : 0;
            scissor.offset.y = draw_cmd->ClipRect.y > 0.0f ? static_cast<int32_t>(draw_cmd->ClipRect.y) : 0;
            scissor.extent.width = static_cast<uint32_t>(draw_cmd->ClipRect.z - draw_cmd->ClipRect.x);
            scissor.extent.height = static_cast<uint32_t>(draw_cmd->ClipRect.w - draw_cmd->ClipRect.y + 1);
            graphics_cmd_buf_->vkCmdSetScissor(0, 1, &scissor);
            graphics_cmd_buf_->vkCmdDrawIndexed(draw_cmd->ElemCount, 1, index_offset, vertex_offset, 0);
          }
          index_offset += draw_cmd->ElemCount;
        }
        vertex_offset += cmd_list->VtxBuffer.Size;
      }
    }
  }

  graphics_cmd_buf_->vkCmdEndRenderPass();

  graphics_cmd_buf_->vkCmdWriteTimestamp(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, query_pool_, 1); // All previous commands must have executed until the end

  graphics_cmd_buf_->vkEndCommandBuffer();

  VkCommandBuffer cmd_buf = graphics_cmd_buf_->command_buffer();
  VkSubmitInfo submit_info {};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.pNext = nullptr;
  submit_info.waitSemaphoreCount = 0;
  submit_info.pWaitSemaphores = nullptr;
  submit_info.pWaitDstStageMask = nullptr;
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &cmd_buf;
  submit_info.signalSemaphoreCount = 1;
  submit_info.pSignalSemaphores = &gbuffer_generation_complete_;
  graphics_queue_->vkQueueSubmit(1, &submit_info, gbuffer_generation_fence_);

  device_->vkDeviceWaitIdle();

  auto end_time = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> duration = end_time - start_time;
  measured_time_ = duration.count();

  uint64_t query_data[2];
  VkResult query_result = device_->vkGetQueryPoolResults(query_pool_, 0, 2, sizeof(query_data), query_data, sizeof(uint64_t), VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);
  if (query_result != VK_SUCCESS) {
    throw;
  }

  // timestampPeriod: nanoseconds per tick
  // division: nanoseconds to milli
  gpu_render_time_ = static_cast<double>(query_data[1] - query_data[0]) * physical_device_properties_.limits.timestampPeriod / 1000000.0;
  if (render_strategy_ == RenderStrategy::DGC) {
    query_result = device_->vkGetQueryPoolResults(query_pool_, 2, 2, sizeof(query_data), query_data, sizeof(uint64_t), VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);
    dgc_generation_time_ = static_cast<double>(query_data[1] - query_data[0]) * physical_device_properties_.limits.timestampPeriod / 1000000.0;
  }
  else {
    dgc_generation_time_ = 0.0;
  }

  // Now that the frame has been processed, we iterate all jobs again to get an
  // idea of the traversal overhead each technique suffers.
  auto traversal_start_time = std::chrono::high_resolution_clock::now();
  if (render_strategy_ == RenderStrategy::Regular) {
    render_cache_->enumerate_all([](RenderCache::JobContext const& c) -> void* { return c.user_data; });
  }
  else {
    // I do this because to reintroduce a branch of `update_indirect_buffers`
    // that was not measured in the `enumerate_all` version. It had a large
    // impact on the traversal time which falsely implied that MDI and DGC had
    // roughly 0.25ms larger overhead than they really did. I don't like this
    // kind of duplication, but it will have to make due.
    RenderCache::Pipeline current_pipeline = RenderCache::Pipeline::Alpha;
    render_cache_->enumerate_all([this, &current_pipeline](RenderCache::JobContext const& c) -> void* {
      // We need something to do inside for the branch to not be optimized away,
      // but we don't want to actually execute it when pipelines start to change.
      // The extra check is to always fail without the compiler figuring it out.
      // This seems to work.

      // The most fair measuring is when `this == 0x0` is removed and we use MDI
      // with one pipeline, which matches manual inspection. That kind of value
      // is used for the thesis.
      if (this == 0x0 && current_pipeline != c.pipeline) {
        std::cout << "hejsan" << std::endl;
      }
      current_pipeline = c.pipeline;
      if (c.change == RenderCache::Change::None) {
        return c.user_data;
      }
      return c.user_data;
    });
  }
  auto traversal_end_time = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> traversal_duration = traversal_end_time - traversal_start_time;
  render_jobs_traversal_time_ = traversal_duration.count();

  // ┌─────────────────────────────────────────────────────────────────┐
  // │  Swapchain image acquisition                                    │
  // └─────────────────────────────────────────────────────────────────┘

  uint32_t image_idx = ~0;
  VkResult result = device_->vkAcquireNextImageKHR(swapchain_->get_swapchain(), UINT64_MAX, image_available_semaphore_, VK_NULL_HANDLE, &image_idx);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Swapchain image acquisition failed (or is suboptimal). Do something appropriate such as resizing the swapchain or whatnot.");
  }

  // ┌─────────────────────────────────────────────────────────────────┐
  // │  Blit rendered image to the swapchain                           │
  // └─────────────────────────────────────────────────────────────────┘

  // Wait for previous frame to finish its use of the blit command buffer. When
  // it has, we reset the fence so that we can sync again after this frame.
  device_->vkWaitForFences(1, &render_fence_, VK_TRUE, UINT64_MAX);
  device_->vkResetFences(1, &render_fence_);

  VkCommandBufferBeginInfo begin_blit {};
  begin_blit.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  begin_blit.pNext = nullptr;
  begin_blit.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  begin_blit.pInheritanceInfo = nullptr;

  blit_swapchain_cmd_buf_->vkBeginCommandBuffer(&begin_blit);

  // Transition swapchain image to a layout suitable for transfer operations.

  if (device_->present_family() != device_->graphics_family()) {
    throw std::runtime_error("Present family differs from graphics family and errors may occur in the presented image. Proper ownership transfer should be done here but I won't bother if not necessary.");
  }

  present_to_transfer_barrier_.srcQueueFamilyIndex = device_->present_family();
  present_to_transfer_barrier_.dstQueueFamilyIndex = device_->graphics_family();
  present_to_transfer_barrier_.image = swapchain_->image(image_idx);

  blit_swapchain_cmd_buf_->vkCmdPipelineBarrier(
    VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
    VK_PIPELINE_STAGE_TRANSFER_BIT,
    0,
    0, nullptr,
    0, nullptr,
    1, &present_to_transfer_barrier_);

  // Perform actual blit.

  VkImageBlit blit_region {};
  blit_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  blit_region.srcSubresource.mipLevel = 0;
  blit_region.srcSubresource.baseArrayLayer = 0;
  blit_region.srcSubresource.layerCount = 1;
  blit_region.srcOffsets[0] = { 0, 0, 0 };
  blit_region.srcOffsets[1] = { static_cast<int32_t>(render_area_.width), static_cast<int32_t>(render_area_.height), 1 };
  blit_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  blit_region.dstSubresource.mipLevel = 0;
  blit_region.dstSubresource.baseArrayLayer = 0;
  blit_region.dstSubresource.layerCount = 1;
  blit_region.dstOffsets[0] = { 0, 0, 0 };
  blit_region.dstOffsets[1] = { static_cast<int32_t>(render_area_.width), static_cast<int32_t>(render_area_.height), 1 };

  blit_swapchain_cmd_buf_->vkCmdBlitImage(
    gbuffer_->get_buf(graphics::deferred_shading::GBuffer::BufferTypes::Albedo).image,
    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
    swapchain_->image(image_idx),
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    1, &blit_region,
    VK_FILTER_LINEAR
  );

  // Transition swapchain image back to a presentable layout.

  transfer_to_present_barrier_.srcQueueFamilyIndex = device_->graphics_family();
  transfer_to_present_barrier_.dstQueueFamilyIndex = device_->present_family();
  transfer_to_present_barrier_.image = swapchain_->image(image_idx);

  blit_swapchain_cmd_buf_->vkCmdPipelineBarrier(
    VK_PIPELINE_STAGE_TRANSFER_BIT,
    VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
    0,
    0, nullptr,
    0, nullptr,
    1, &transfer_to_present_barrier_);

  blit_swapchain_cmd_buf_->vkEndCommandBuffer();

  // Submit the blit command buffer.

  std::array<VkSemaphore, 2> blit_wait_semaphores = { image_available_semaphore_, gbuffer_generation_complete_ };
  std::array<VkPipelineStageFlags, blit_wait_semaphores.size()> blit_wait_stages = { VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT };
  {
    VkCommandBuffer cmd_buf = blit_swapchain_cmd_buf_->command_buffer();
    VkSubmitInfo blit_swapchain_submission {};
    blit_swapchain_submission.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    blit_swapchain_submission.pNext = nullptr;
    blit_swapchain_submission.waitSemaphoreCount = static_cast<uint32_t>(blit_wait_semaphores.size());
    blit_swapchain_submission.pWaitSemaphores = blit_wait_semaphores.data();
    blit_swapchain_submission.pWaitDstStageMask = blit_wait_stages.data();
    blit_swapchain_submission.commandBufferCount = 1;
    blit_swapchain_submission.pCommandBuffers = &cmd_buf;
    blit_swapchain_submission.signalSemaphoreCount = 1;
    blit_swapchain_submission.pSignalSemaphores = &blit_swapchain_complete_;
    graphics_queue_->vkQueueSubmit(1, &blit_swapchain_submission, render_fence_);
  }

  // ┌─────────────────────────────────────────────────────────────────┐
  // │  Present swapchain image                                        │
  // └─────────────────────────────────────────────────────────────────┘

  std::array<VkSwapchainKHR, 1> swapchains = { swapchain_->get_swapchain() };
  VkPresentInfoKHR present_info {};
  present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  present_info.pNext = nullptr;
  present_info.waitSemaphoreCount = 1;
  present_info.pWaitSemaphores = &blit_swapchain_complete_;
  present_info.swapchainCount = static_cast<uint32_t>(swapchains.size());
  present_info.pSwapchains = swapchains.data();
  present_info.pImageIndices = &image_idx;
  present_info.pResults = nullptr;

  result = present_queue_->vkQueuePresentKHR(&present_info);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Swapchain present error (or suboptimal). Do something appropriate such as resizing the swapchain or whatnot.");
  }
}

void Renderer::use_matrices(DirectX::CXMMATRIX view, DirectX::CXMMATRIX proj) {
  DirectX::XMStoreFloat4x4(&view_, view);
  DirectX::XMStoreFloat4x4(&proj_, DirectX::XMMatrixScaling(1.0f, -1.0f, 1.0f) * proj);
}

void Renderer::create_instance() {
  vk::InstanceBuilder builder;

  // TODO: Disabled because it crashes vkRegisterObjectsNVX
  //builder.use_layer("VK_LAYER_LUNARG_standard_validation");

  builder.use_extension("VK_EXT_debug_report");
  builder.use_extension("VK_KHR_surface");
#if defined(_WIN32)
  builder.use_extension("VK_KHR_win32_surface");
#else
#error "Unsupported OS"
#endif

  instance_ = builder.build(vk_globals_.get());
}

void Renderer::borrow_render_cache(std::function<void(RenderCache& cache)> const& provide) {
  provide(*render_cache_);
}

void Renderer::update_transform(uint32_t render_job, DirectX::CXMMATRIX transform) {
  render_jobs_descriptor_set_->update_data(*device_, render_job, [&transform](RenderJobsDescriptorSet::RenderJobData& data) {
    DirectX::XMStoreFloat4x4(&data.transform, transform);
  });
}

// TODO: Replace with a method to begin/end measure sessions
void Renderer::should_render_ui(bool should) {
  //render_ui_ = should;
}

double Renderer::measured_time() const {
  return measured_time_;
}

double Renderer::gpu_time() const {
  return gpu_render_time_;
}

double Renderer::dgc_generation_time() const {
  return dgc_generation_time_;
}

double Renderer::render_jobs_traversal_time() const {
  return render_jobs_traversal_time_;
}

void Renderer::use_render_strategy(RenderStrategy strategy) {
  render_strategy_ = strategy;
  render_cache_->dirtify_all();
}

Renderer::RenderStrategy Renderer::current_strategy() const {
  return render_strategy_;
}

void Renderer::use_flush_behavior(Flush flush) {
  flush_behavior_ = flush;
}

Renderer::Flush Renderer::current_flush_behavior() const {
  return flush_behavior_;
}

void Renderer::create_debug_callback() {
  VkDebugReportCallbackCreateInfoEXT callback_info {};
  callback_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
  callback_info.pNext = nullptr;
  callback_info.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
  callback_info.pUserData = nullptr;
  callback_info.pfnCallback = [](VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData) -> VkBool32 {
    std::cout << pLayerPrefix << " " << pMessage << std::endl;
    return VK_FALSE;
  };

  VkResult result = instance_->vkCreateDebugReportCallbackEXT(&callback_info, nullptr, &debug_callback_);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Could not create debug callback.");
  }
}

void Renderer::create_surface(HWND hwnd) {
  surface_ = vk::PresentationSurface::for_win32(hwnd, instance_);
}

void Renderer::create_device() {
  vk::DeviceBuilder builder;
  builder.use_extension("VK_KHR_swapchain");
  builder.use_extension("VK_KHR_shader_draw_parameters");
  builder.use_extension("VK_NVX_device_generated_commands");
  device_ = builder.build(instance_.get());
  graphics_queue_ = device_->graphics_queue();
  compute_queue_ = device_->compute_queue();
  present_queue_ = device_->present_queue();

  device_->physical_device()->vkGetPhysicalDeviceProperties(&physical_device_properties_);
}

void Renderer::create_swapchain() {
  swapchain_.reset(new vk::Swapchain(device_, surface_));
}

void Renderer::create_command_pools_and_buffers() {
  stable_graphics_cmd_pool_ = vk::CommandPool::make_stable(device_->graphics_family(), device_);
  graphics_cmd_buf_ = stable_graphics_cmd_pool_->allocate_primary();

  transient_graphics_cmd_pool_ = vk::CommandPool::make_transient(device_->graphics_family(), device_);
  dgc_cmd_buf_ = transient_graphics_cmd_pool_->allocate_secondary();
  blit_swapchain_cmd_buf_ = transient_graphics_cmd_pool_->allocate_primary();
}

void Renderer::create_samplers() {
  VkSamplerCreateInfo sampler_info = {};
  sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  sampler_info.pNext = nullptr;
  sampler_info.flags = 0;
  sampler_info.magFilter = VK_FILTER_LINEAR;
  sampler_info.minFilter = VK_FILTER_LINEAR;
  sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  sampler_info.mipLodBias = 0.0f;
  sampler_info.anisotropyEnable = VK_FALSE;
  sampler_info.maxAnisotropy = 1.0f;
  sampler_info.compareEnable = VK_FALSE;
  sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
  sampler_info.minLod = 0.0f;
  sampler_info.maxLod = 0.0f;
  sampler_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
  sampler_info.unnormalizedCoordinates = VK_FALSE;
  VkResult result = device_->vkCreateSampler(&sampler_info, nullptr, &gui_font_sampler_);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Could not create sampler for GUI font.");
  }
}

void Renderer::create_render_pass() {
  const uint32_t buf_count = static_cast<uint32_t>(graphics::deferred_shading::GBuffer::BufferTypes::BufferCount);

  vk::RenderPassBuilder builder;
  for (uint32_t i = 0; i < buf_count; ++i) {
    builder.attachment(vk::RenderPassAttachment::c_clear_store(gbuffer_->buffers()[i].format));
  }
  builder.attachment(vk::RenderPassAttachment::d_clear_store(VK_FORMAT_D32_SFLOAT));
  
  // Fill gbuffer subpass
  builder.graphics_subpass([buf_count](vk::Subpass& subpass) {
    for (uint32_t i = 0; i < buf_count; ++i) {
      subpass.color_attachment(i, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    }
    subpass.depth_stencil_attachment(buf_count, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
  });

  // Render DGC subpass
  static_assert(buf_count == 2); // Ordinary buffers. There is also an additional depth
  builder.graphics_subpass([](vk::Subpass& subpass) {
    subpass.color_attachment(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    subpass.color_attachment(1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    subpass.depth_stencil_attachment(2, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL); // Depth
  });

  // Render GUI to albedo pass
  static_assert(buf_count == 2);
  builder.graphics_subpass([](vk::Subpass& subpass) {
    subpass.color_attachment(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    subpass.preserve_attachment(1); // Normal
    subpass.depth_stencil_attachment(2, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL); // Depth
  });

  VkSubpassDependency dependency {};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
  dependency.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
  dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

  builder.subpass_dependency(dependency);

  dependency.srcSubpass = 0;
  dependency.dstSubpass = 1;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
  dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
  dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

  builder.subpass_dependency(dependency);

  dependency.srcSubpass = 1;
  dependency.dstSubpass = 2;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
  dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
  dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

  builder.subpass_dependency(dependency);

  dependency.srcSubpass = 2;
  dependency.dstSubpass = VK_SUBPASS_EXTERNAL;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
  dependency.dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
  dependency.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
  dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

  builder.subpass_dependency(dependency);

  gbuffer_render_pass_ = builder.build(*device_);
}

void Renderer::create_framebuffer() {
  const uint32_t buf_count = static_cast<uint32_t>(graphics::deferred_shading::GBuffer::BufferTypes::BufferCount);

  std::array<VkImageView, buf_count + 1> attachments = {};
  for (uint32_t i = 0; i < buf_count; ++i) {
    attachments[i] = gbuffer_->buffers()[i].view;
  }
  attachments[buf_count] = depth_buffer_->get_view();

  VkFramebufferCreateInfo framebuffer_info {};
  framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  framebuffer_info.pNext = nullptr;
  framebuffer_info.flags = 0;
  framebuffer_info.renderPass = gbuffer_render_pass_;
  framebuffer_info.attachmentCount = attachments.size();
  framebuffer_info.pAttachments = attachments.data();
  framebuffer_info.width = gbuffer_->width();
  framebuffer_info.height = gbuffer_->height();
  framebuffer_info.layers = 1;

  VkResult result = device_->vkCreateFramebuffer(&framebuffer_info, nullptr, &framebuffer_);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Could not create deferred shading framebuffer.");
  }
}

void Renderer::create_shaders() {
  auto create_shader = [this](std::string const& shader_file, VkShaderModule& shader_module) {
    std::ifstream file { shader_file, std::ios::ate | std::ios::binary };
    if (!file) {
      throw std::runtime_error("Could not open shader file.");
    }

    auto size = file.tellg();
    file.seekg(0, std::ios::beg);

    auto spirv = std::unique_ptr<char[]>(new char[size]);

    file.read(spirv.get(), size);
    file.close();

    VkShaderModuleCreateInfo shader_info {};
    shader_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shader_info.pNext = nullptr;
    shader_info.flags = 0;
    shader_info.codeSize = size;
    shader_info.pCode = reinterpret_cast<uint32_t const*>(spirv.get());

    VkResult result = device_->vkCreateShaderModule(&shader_info, nullptr, &shader_module);
    if (result != VK_SUCCESS) {
      throw std::runtime_error("Could not create shader module.");
    }
  };

  create_shader("shaders/fullscreen-triangle-vs.spv", fullscreen_triangle_vs_);
  create_shader("shaders/fill-gbuffer-vs.spv", fill_gbuffer_vs_);
  create_shader("shaders/fill-gbuffer-ps.spv", fill_gbuffer_fs_);
  create_shader("shaders/gui-vs.spv", gui_vs_);
  create_shader("shaders/gui-ps.spv", gui_fs_);
}

void Renderer::create_pipeline() {
  vk::PipelineLayoutBuilder layout_builder;
  layout_builder.push_constant(VK_SHADER_STAGE_VERTEX_BIT, 0, 2 * sizeof(DirectX::XMFLOAT4X4) + sizeof(uint32_t));
  layout_builder.descriptor_layout(render_jobs_descriptor_set_->layout().vulkan_handle());
  gbuffer_pipeline_layout_ = layout_builder.build(*device_);

  // Mesh rendering pipelines (4 total: 2x (wireframe/solid) regular/mdi + 2x (wireframe/solid) dgc)
  {
    vk::PipelineBuilder pipeline_builder;

    pipeline_builder.shader_stage(VK_SHADER_STAGE_VERTEX_BIT, fill_gbuffer_vs_);

    struct SpecializationData {
      uint32_t using_dgc;
    } spec_data;
    spec_data.using_dgc = 0;
    pipeline_builder.shader_specialization_data(&spec_data, sizeof(spec_data));
    pipeline_builder.shader_specialization_map(0, 0, sizeof(SpecializationData::using_dgc));

    pipeline_builder.shader_stage(VK_SHADER_STAGE_FRAGMENT_BIT, fill_gbuffer_fs_);

    pipeline_builder.vertex_layout([](auto& layout) {
      layout.stream(0, 12 + 8 + 12, VK_VERTEX_INPUT_RATE_VERTEX); // pos + tex + normal
      layout.attribute(0, VK_FORMAT_R32G32B32_SFLOAT, 0);
      layout.attribute(1, VK_FORMAT_R32G32_SFLOAT, 12);
      layout.attribute(2, VK_FORMAT_R32G32B32_SFLOAT, 20);
    });

    pipeline_builder.ia_triangle_list();
    pipeline_builder.vp_dynamic();
    pipeline_builder.rs_fill_cull_back();
    pipeline_builder.ms_none();
    pipeline_builder.ds_enabled();
    pipeline_builder.bs_none(2);
    pipeline_builder.dynamic_state({
      VK_DYNAMIC_STATE_VIEWPORT,
      VK_DYNAMIC_STATE_SCISSOR
      });

    pipeline_regular_mdi_solid_ = pipeline_builder.build(*device_, gbuffer_pipeline_layout_, gbuffer_render_pass_, 0);

    spec_data.using_dgc = 1;
    pipeline_dgc_solid_ = pipeline_builder.build(*device_, gbuffer_pipeline_layout_, gbuffer_render_pass_, 1);

    pipeline_builder.rs_wireframe_cull_back();

    spec_data.using_dgc = 0;
    pipeline_regular_mdi_wireframe_ = pipeline_builder.build(*device_, gbuffer_pipeline_layout_, gbuffer_render_pass_, 0);

    spec_data.using_dgc = 1;
    pipeline_dgc_wireframe_ = pipeline_builder.build(*device_, gbuffer_pipeline_layout_, gbuffer_render_pass_, 1);
  }

  vk::PipelineLayoutBuilder gui_layout_builder;
  gui_layout_builder.push_constant(VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(float) * 4); // GUI scaling and translation
  gui_layout_builder.descriptor_layout(gui_descriptor_set_->layout().vulkan_handle());
  gui_pipeline_layout_ = gui_layout_builder.build(*device_);

  vk::PipelineBuilder gui_pipeline_builder;

  gui_pipeline_builder.shader_stage(VK_SHADER_STAGE_VERTEX_BIT, gui_vs_);
  gui_pipeline_builder.shader_stage(VK_SHADER_STAGE_FRAGMENT_BIT, gui_fs_);

  gui_pipeline_builder.vertex_layout([](auto& layout) {
    layout.stream(0, sizeof(ImDrawVert), VK_VERTEX_INPUT_RATE_VERTEX);
    layout.attribute(0, VK_FORMAT_R32G32_SFLOAT, (size_t)(&((ImDrawVert*)0)->pos));
    layout.attribute(1, VK_FORMAT_R32G32_SFLOAT, (size_t)(&((ImDrawVert*)0)->uv));
    layout.attribute(2, VK_FORMAT_R8G8B8A8_UNORM, (size_t)(&((ImDrawVert*)0)->col));
  });

  gui_pipeline_builder.ia_triangle_list();
  gui_pipeline_builder.vp_dynamic();
  gui_pipeline_builder.rs_fill_cull_none();
  gui_pipeline_builder.ms_none();
  gui_pipeline_builder.ds_none();
  gui_pipeline_builder.bs_hardcoded_single_attachment_alpha_blend_because_lazy();
  gui_pipeline_builder.dynamic_state({
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR
  });

  gui_pipeline_ = gui_pipeline_builder.build(*device_, gui_pipeline_layout_, gbuffer_render_pass_, 2);
}

void Renderer::create_synchronization_primitives() {
  image_available_semaphore_ = device_->create_semaphore();
  blit_swapchain_complete_ = device_->create_semaphore();
  gbuffer_generation_complete_ = device_->create_semaphore();
  render_fence_ = device_->create_fence(true);
  gbuffer_generation_fence_ = device_->create_fence(true);
}

void Renderer::configure_barrier_structs() {
  present_to_transfer_barrier_.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  present_to_transfer_barrier_.pNext = nullptr;
  present_to_transfer_barrier_.srcAccessMask = 0;
  present_to_transfer_barrier_.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  present_to_transfer_barrier_.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  present_to_transfer_barrier_.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  present_to_transfer_barrier_.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  present_to_transfer_barrier_.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  present_to_transfer_barrier_.image = VK_NULL_HANDLE;
  present_to_transfer_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  present_to_transfer_barrier_.subresourceRange.baseMipLevel = 0;
  present_to_transfer_barrier_.subresourceRange.levelCount = 1;
  present_to_transfer_barrier_.subresourceRange.baseArrayLayer = 0;
  present_to_transfer_barrier_.subresourceRange.layerCount = 1;

  transfer_to_present_barrier_.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  transfer_to_present_barrier_.pNext = nullptr;
  transfer_to_present_barrier_.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  transfer_to_present_barrier_.dstAccessMask = 0;
  transfer_to_present_barrier_.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  transfer_to_present_barrier_.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  transfer_to_present_barrier_.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  transfer_to_present_barrier_.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  transfer_to_present_barrier_.image = VK_NULL_HANDLE;
  transfer_to_present_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  transfer_to_present_barrier_.subresourceRange.baseMipLevel = 0;
  transfer_to_present_barrier_.subresourceRange.levelCount = 1;
  transfer_to_present_barrier_.subresourceRange.baseArrayLayer = 0;
  transfer_to_present_barrier_.subresourceRange.layerCount = 1;
}

void Renderer::create_vertex_buffer() {
  // Here we build a single vertex buffer with various hardcoded meshes in it.
  // For ease of implementation they all use the same vertex layout and simply
  // duplicates vertices instead of using indices.

  std::vector<Vertex> vertices;
  generate_box_vertices(vertices);
  load_sphere(vertices);
  generate_triangles(vertices);

  vertex_buffer_.reset(new vk::Buffer(*device_, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, sizeof(Vertex) * vertices.size()));

  // Temporarilly creating staging buffers like this is not the best way
  // to go, but works well enough for my purposes. My stuff will be hard-
  // coded so I know what I need, but in a propery scenario is should be
  // reused for other buffers.
  vk::Buffer staging { *device_, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, sizeof(Vertex) * vertices.size() };

  void* mapped_data = nullptr;
  device_->vkMapMemory(staging.vulkan_memory_handle(), 0, VK_WHOLE_SIZE, 0, &mapped_data);

  memcpy(mapped_data, vertices.data(), sizeof(Vertex) * vertices.size());

  VkMappedMemoryRange flush_range {};
  flush_range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
  flush_range.pNext = nullptr;
  flush_range.memory = staging.vulkan_memory_handle();
  flush_range.offset = 0;
  flush_range.size = VK_WHOLE_SIZE;
  device_->vkFlushMappedMemoryRanges(1, &flush_range);

  device_->vkUnmapMemory(staging.vulkan_memory_handle());

  VkCommandBufferBeginInfo begin_info {};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  begin_info.pNext = nullptr;
  begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  begin_info.pInheritanceInfo = nullptr;

  graphics_cmd_buf_->vkBeginCommandBuffer(&begin_info);

  VkBufferCopy copy_region {};
  copy_region.srcOffset = 0;
  copy_region.dstOffset = 0;
  copy_region.size = sizeof(Vertex) * vertices.size();

  graphics_cmd_buf_->vkCmdCopyBuffer(staging.vulkan_buffer_handle(), vertex_buffer_->vulkan_buffer_handle(), 1, &copy_region);

  graphics_cmd_buf_->vkEndCommandBuffer();

  VkCommandBuffer cmd_buf = graphics_cmd_buf_->command_buffer();
  VkSubmitInfo submit_info {};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.pNext = nullptr;
  submit_info.waitSemaphoreCount = 0;
  submit_info.pWaitSemaphores = nullptr;
  submit_info.pWaitDstStageMask = nullptr;
  submit_info.signalSemaphoreCount = 0;
  submit_info.pSignalSemaphores = nullptr;
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &cmd_buf;
  graphics_queue_->vkQueueSubmit(1, &submit_info, VK_NULL_HANDLE);
  graphics_queue_->vkQueueWaitIdle();

  staging.destroy(*device_);
}

void Renderer::generate_box_vertices(std::vector<Vertex>& vertices) {
  const float half = 0.7f;
  vertices.reserve(vertices.size() + 36);
  vertices.insert(vertices.end(), {
    // Front
    { { -half, -half, -half }, { 0.0f, 1.0f }, { 0.0f, 0.0f, -1.0f } },
    { { -half, +half, -half }, { 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } },
    { { +half, +half, -half }, { 1.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } },
    { { +half, +half, -half }, { 1.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } },
    { { +half, -half, -half }, { 1.0f, 1.0f }, { 0.0f, 0.0f, -1.0f } },
    { { -half, -half, -half }, { 0.0f, 1.0f }, { 0.0f, 0.0f, -1.0f } },
    // Right
    { { +half, -half, -half }, { 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f } },
    { { +half, +half, -half }, { 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
    { { +half, +half, +half }, { 1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
    { { +half, +half, +half }, { 1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
    { { +half, -half, +half }, { 1.0f, 1.0f }, { 1.0f, 0.0f, 0.0f } },
    { { +half, -half, -half }, { 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f } },
    // Back
    { { +half, -half, +half }, { 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f } },
    { { +half, +half, +half }, { 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
    { { -half, +half, +half }, { 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
    { { -half, +half, +half }, { 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
    { { -half, -half, +half }, { 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f } },
    { { +half, -half, +half }, { 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f } },
    // Left
    { { -half, -half, +half }, { 0.0f, 1.0f }, { -1.0f, 0.0f, 0.0f } },
    { { -half, +half, +half }, { 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f } },
    { { -half, +half, -half }, { 1.0f, 0.0f }, { -1.0f, 0.0f, 0.0f } },
    { { -half, +half, -half }, { 1.0f, 0.0f }, { -1.0f, 0.0f, 0.0f } },
    { { -half, -half, -half }, { 1.0f, 1.0f }, { -1.0f, 0.0f, 0.0f } },
    { { -half, -half, +half }, { 0.0f, 1.0f }, { -1.0f, 0.0f, 0.0f } },
    // Top
    { { -half, +half, -half }, { 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } },
    { { -half, +half, +half }, { 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
    { { +half, +half, +half }, { 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
    { { +half, +half, +half }, { 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
    { { +half, +half, -half }, { 1.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } },
    { { -half, +half, -half }, { 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } },
    // Bottom
    { { -half, -half, +half }, { 0.0f, 1.0f }, { 0.0f, -1.0f, 0.0f } },
    { { -half, -half, -half }, { 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } },
    { { +half, -half, -half }, { 1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } },
    { { +half, -half, -half }, { 1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } },
    { { +half, -half, +half }, { 1.0f, 1.0f }, { 0.0f, -1.0f, 0.0f } },
    { { -half, -half, +half }, { 0.0f, 1.0f }, { 0.0f, -1.0f, 0.0f } },
  });
}

void Renderer::load_sphere(std::vector<Vertex>& vertices) {
  OBJLoader loader;
  Mesh* mesh = loader.Load("assets/sphere.obj");
  mesh->FlipPositionZ();
  mesh->FlipNormals();
  mesh->InvertV();

  interleave_vertex_data(mesh, vertices);
}

void Renderer::generate_triangles(std::vector<Vertex>& vertices) {
  const float half = 0.7f;
  vertices.reserve(vertices.size() + 6);
  vertices.insert(vertices.end(), {
    // Pointing up
    { { -half, -half, -half }, { 0.0f, 1.0f }, { 0.0f, 0.0f, -1.0f } },
    { { 0.0f, +half, -half }, { 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } },
    { { +half, -half, -half }, { 1.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } },
    // Pointing down
    { { -half, +half, -half }, { 0.0f, 1.0f }, { 0.0f, 0.0f, -1.0f } },
    { { +half, +half, -half }, { 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } },
    { { 0.0f, -half, -half }, { 1.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } },
  });
}

void Renderer::interleave_vertex_data(Mesh* mesh, std::vector<Vertex>& vertices) {
  // I don't really use the index buffer, but just duplicate vertices. This is
  // not a particularly good way to render meshes, but the focus here is to
  // efficiently dispatch draw calls, not the rendering itself.
  vertices.reserve(vertices.size() + mesh->IndexCount());

  auto& positions = mesh->AttributeData(mesh->FindStream(Mesh::AttributeType::Position));
  auto position_indices = mesh->AttributeIndices(mesh->FindStream(Mesh::AttributeType::Position));
  auto& texcoords = mesh->AttributeData(mesh->FindStream(Mesh::AttributeType::TexCoord));
  auto texcoord_indices = mesh->AttributeIndices(mesh->FindStream(Mesh::AttributeType::TexCoord));
  auto& normals = mesh->AttributeData(mesh->FindStream(Mesh::AttributeType::Normal));
  auto normal_indices = mesh->AttributeIndices(mesh->FindStream(Mesh::AttributeType::Normal));

  for (uint32_t vertex = 0, i = 0; i < mesh->IndexCount(); ++i) {
    Vertex v;
    v.pos = ((DirectX::XMFLOAT3*)positions.data())[position_indices[i]];
    v.tex = ((DirectX::XMFLOAT2*)texcoords.data())[texcoord_indices[i]];
    v.nor = ((DirectX::XMFLOAT3*)normals.data())[normal_indices[i]];
    vertices.push_back(v);

    // Every third vertex (triangle) we change winding order.
    if (vertex == 2) {
      std::swap(*vertices.rbegin(), *(vertices.rbegin() + 2));
      vertex = 0;
    }
    else {
      vertex++;
    }
  }
}

void Renderer::create_descriptor_sets() {
  vk::DescriptorPoolBuilder builder;
  builder.reserve(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);
  builder.reserve(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1);
  builder.max_sets(2);
  descriptor_pool_.reset(new vk::DescriptorPool { *device_, builder });

  render_jobs_descriptor_set_.reset(new RenderJobsDescriptorSet { *device_, *descriptor_pool_ });
  gui_descriptor_set_.reset(new GuiDescriptorSet{ *device_, *descriptor_pool_, gui_font_sampler_});
}

void Renderer::create_indirect_buffer() {
  indirect_buffer_.reset(new vk::Buffer { *device_, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, sizeof(VkDrawIndirectCommand) * g_draw_call_count });
  device_->vkMapMemory(indirect_buffer_->vulkan_memory_handle(), 0, sizeof(VkDrawIndirectCommand) * g_draw_call_count, 0, reinterpret_cast<void**>(&mapped_indirect_buffer_));
}

// TODO: The next step is to update the indirect buffer incrementally.

// To allow rendering just the number of meshes that we have recorded, we need
// to pack the draw calls at the beginning of the buffer. This implies that we
// can't just invalidate a removed draw call by setting its instance count to
// zero because that "no-op draw" is included in the count, meaning that the
// last object would not be included. Instead we move the last draw call in its
// place, which is fine because we use base instance instead of drawid for
// mapping to instance data in the shader so there's nothing to synchronize.
// This will not be an issue for me (I will just rewrite the same calls and not
// delete them), at least not at first, but is mentioned for completeness.
// Note that AZDO says base instance is not visible for us, so it's probably
// something new or Vulkan-specific in shader_draw_parameters.

// Moving the last element can be tricky for the future, though. If that job
// is later deleted in the cache, we need to find it in the indirect buffer.
// We can probably use tracking information that is returned to the cache via
// the lambda when jobs are added. This tracking information is provided on
// subsequent updates for the render job. If the tracking information is the
// location in the indirect buffer, we can easily synchronize the indirect
// buffer on updates later.

// A dirtify method could be used on the cache to indicate that we want to
// trigger a record for the draw call. This would be unnecessary in a real
// application, but allows us to easily measure performance impact of various
// amounts of incremental changes.

#pragma push_macro("max")
#undef max
void Renderer::update_indirect_buffer(std::function<void(uint32_t job, VkPipeline pipeline)> const& pipeline_switch, bool dgc) {
  RenderCache::Pipeline current_pipeline = RenderCache::Pipeline::Alpha;

  render_cache_->enumerate_all([this, &current_pipeline, &pipeline_switch, dgc](RenderCache::JobContext const& job_context) -> void* {
    if (current_pipeline != job_context.pipeline) {
      pipeline_switch(job_context.job, current_pipeline == RenderCache::Pipeline::Alpha ? pipeline_regular_mdi_solid_ : pipeline_regular_mdi_wireframe_);
    }

    current_pipeline = job_context.pipeline;

    if (job_context.change == RenderCache::Change::None) {
      return job_context.user_data;
    }

    assert(job_context.change == RenderCache::Change::Modify);

    size_t indirect_buffer_element = job_context.job;

    VkDrawIndirectCommand* indirect_command = mapped_indirect_buffer_ + indirect_buffer_element;
    //indirect_command->vertexCount = job_context.object_type == RenderObject::Box ? 36 : 2160;
    indirect_command->vertexCount = 3;
    indirect_command->instanceCount = 1;
    indirect_command->firstVertex = job_context.object_type == RenderObject::Box ? 2196 : 2199;
    indirect_command->firstInstance = job_context.job;

    if (dgc) {
      uint32_t* pipeline = mapped_dgc_pipeline_parameters_ + indirect_buffer_element;
      *pipeline = job_context.pipeline == RenderCache::Pipeline::Alpha ? 0 : 1;

      Push* push = mapped_dgc_push_constants_ + indirect_buffer_element;
      push->table_entry = 0;
      push->actual_data = job_context.job;
    }

    if (flush_behavior_ == Flush::Individual) {
      std::array<VkMappedMemoryRange, 3> flush_ranges {};
      flush_ranges[0].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
      flush_ranges[0].pNext = nullptr;
      flush_ranges[0].memory = indirect_buffer_->vulkan_memory_handle();
      flush_ranges[0].offset = indirect_buffer_element * sizeof(VkDrawIndirectCommand);
      flush_ranges[0].size = sizeof(VkDrawIndirectCommand);

      if (dgc) {
        flush_ranges[1].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        flush_ranges[1].pNext = nullptr;
        flush_ranges[1].memory = dgc_pipeline_parameters_->vulkan_memory_handle();
        flush_ranges[1].offset = indirect_buffer_element * sizeof(uint32_t);
        flush_ranges[1].size = sizeof(uint32_t);
        flush_ranges[2].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        flush_ranges[2].pNext = nullptr;
        flush_ranges[2].memory = dgc_push_constants_->vulkan_memory_handle();
        flush_ranges[2].offset = indirect_buffer_element * sizeof(Push);
        flush_ranges[2].size = sizeof(Push);
      }

      device_->vkFlushMappedMemoryRanges(dgc ? 3 : 1, flush_ranges.data());
    }

    return reinterpret_cast<void*>(indirect_buffer_element);
  });

  pipeline_switch(g_draw_call_count, current_pipeline == RenderCache::Pipeline::Alpha ? pipeline_regular_mdi_solid_ : pipeline_regular_mdi_wireframe_);

  if (flush_behavior_ == Flush::Once) {
    std::array<VkMappedMemoryRange, 3> flush_ranges {};
    flush_ranges[0].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    flush_ranges[0].pNext = nullptr;
    flush_ranges[0].memory = indirect_buffer_->vulkan_memory_handle();
    flush_ranges[0].offset = 0;
    flush_ranges[0].size = sizeof(VkDrawIndirectCommand) * g_draw_call_count;

    if (dgc) {
      flush_ranges[1].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
      flush_ranges[1].pNext = nullptr;
      flush_ranges[1].memory = dgc_pipeline_parameters_->vulkan_memory_handle();
      flush_ranges[1].offset = 0;
      flush_ranges[1].size = sizeof(uint32_t) * g_draw_call_count;
      flush_ranges[2].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
      flush_ranges[2].pNext = nullptr;
      flush_ranges[2].memory = dgc_push_constants_->vulkan_memory_handle();
      flush_ranges[2].offset = 0;
      flush_ranges[2].size = sizeof(Push) * g_draw_call_count;
    }

    device_->vkFlushMappedMemoryRanges(dgc ? 3 : 1, flush_ranges.data());
  }
}
#pragma pop_macro("max");

void Renderer::initialize_imgui() {
  // Color scheme
  //ImGuiStyle& style = ImGui::GetStyle();
  //style.Colors[ImGuiCol_TitleBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.6f);
  //style.Colors[ImGuiCol_TitleBgActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
  //style.Colors[ImGuiCol_MenuBarBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
  //style.Colors[ImGuiCol_Header] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
  //style.Colors[ImGuiCol_CheckMark] = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
  ImGui::StyleColorsDark();

  // Load font
  // ImGui uses the default font if none is loaded. ImGui::PushFont / ImGui::PopFont
  // Fonts are rasterized with ImFontAtlas::Build
  //io.Fonts->AddFontDefault();
  //io.Fonts->AddFontFromFileTTF("assets/fonts/Roboto-Medium.ttf", 16.0f);
  //ImFont* saved_font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());

  create_imgui_font_texture();
}

void Renderer::create_imgui_font_texture() {
  ImGuiIO& io = ImGui::GetIO();
  unsigned char* texels;
  int width, height;
  io.Fonts->GetTexDataAsRGBA32(&texels, &width, &height);
  size_t upload_size = width * height * 4 * sizeof(char);

  // Create image
  {
    VkImageCreateInfo image_info {};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.pNext = nullptr;
    image_info.flags = 0;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_info.extent.width = width;
    image_info.extent.height = height;
    image_info.extent.depth = 1;
    image_info.mipLevels = 1;
    image_info.arrayLayers = 1;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_info.queueFamilyIndexCount = 0;
    image_info.pQueueFamilyIndices = nullptr;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkResult result = device_->vkCreateImage(&image_info, nullptr, &gui_font_image_);
    if (result != VK_SUCCESS) {
      throw std::runtime_error("Could not create GUI font image.");
    }
  }

  // Allocate and bind image memory
  {
    VkMemoryRequirements mem_req {};
    device_->vkGetImageMemoryRequirements(gui_font_image_, &mem_req);

    VkMemoryAllocateInfo alloc_info {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.pNext = nullptr;
    alloc_info.allocationSize = mem_req.size;
    alloc_info.memoryTypeIndex = 0;

    VkMemoryPropertyFlagBits desired_props = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    auto const& mem_props = device_->physical_device()->memory_properties();
    for (uint32_t i = 0; i < mem_props.memoryTypeCount; ++i) {
      // Current memory type (i) suitable and the memory type has desired properties.
      if ((mem_req.memoryTypeBits & (1 << i)) && ((mem_props.memoryTypes[i].propertyFlags & desired_props) == desired_props)) {
        alloc_info.memoryTypeIndex = i;
        break;
      }
    }

    VkResult result = device_->vkAllocateMemory(&alloc_info, nullptr, &gui_font_image_memory_);
    if (result != VK_SUCCESS) {
      throw std::runtime_error("Could not allocate memory for GUI font image.");
    }

    result = device_->vkBindImageMemory(gui_font_image_, gui_font_image_memory_, 0);
    if (result != VK_SUCCESS) {
      throw std::runtime_error("Could not bind GUI font image to memory.");
    }
  }

  // Create image view
  {
    VkImageViewCreateInfo view_info {};
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.pNext = nullptr;
    view_info.flags = 0;
    view_info.image = gui_font_image_;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    view_info.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
    view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;
    VkResult result = device_->vkCreateImageView(&view_info, nullptr, &gui_font_image_view_);
    if (result != VK_SUCCESS) {
      throw std::runtime_error("Could not create GUI font image view.");
    }
  }

  // Make the GUI descriptor set use the font image
  {
    gui_descriptor_set_->use_font_image(*device_, gui_font_image_view_, gui_font_sampler_);
  }

  // Create a temporary upload buffer
  std::unique_ptr<vk::Buffer> upload_buffer { new vk::Buffer{*device_, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, upload_size} };

  // Upload font image to buffer
  {
    void* mapped_data { nullptr };
    device_->vkMapMemory(upload_buffer->vulkan_memory_handle(), 0, upload_size, 0, &mapped_data);
    memcpy(mapped_data, texels, upload_size);

    VkMappedMemoryRange flush_range {};
    flush_range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    flush_range.pNext = nullptr;
    flush_range.memory = upload_buffer->vulkan_memory_handle();
    flush_range.offset = 0;
    flush_range.size = upload_size;
    device_->vkFlushMappedMemoryRanges(1, &flush_range);
    device_->vkUnmapMemory(upload_buffer->vulkan_memory_handle());
  }

  VkCommandBufferBeginInfo begin_info {};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  begin_info.pNext = nullptr;
  begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  begin_info.pInheritanceInfo = nullptr;
  graphics_cmd_buf_->vkBeginCommandBuffer(&begin_info);

  // Copy to device local image
  {
    VkImageMemoryBarrier copy_barrier {};
    copy_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    copy_barrier.pNext = nullptr;
    copy_barrier.srcAccessMask = 0;
    copy_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    copy_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    copy_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    copy_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    copy_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    copy_barrier.image = gui_font_image_;
    copy_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_barrier.subresourceRange.baseMipLevel = 0;
    copy_barrier.subresourceRange.levelCount = 1;
    copy_barrier.subresourceRange.baseArrayLayer = 0;
    copy_barrier.subresourceRange.layerCount = 1;
    graphics_cmd_buf_->vkCmdPipelineBarrier(
      VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
      0,
      0, nullptr,
      0, nullptr,
      1, &copy_barrier
    );

    VkBufferImageCopy copy_region {};
    copy_region.bufferOffset = 0;
    copy_region.bufferRowLength = 0;
    copy_region.bufferImageHeight = 0;
    copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.imageSubresource.mipLevel = 0;
    copy_region.imageSubresource.baseArrayLayer = 0;
    copy_region.imageSubresource.layerCount = 1;
    copy_region.imageOffset = { 0, 0, 0 };
    copy_region.imageExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1 };
    graphics_cmd_buf_->vkCmdCopyBufferToImage(upload_buffer->vulkan_buffer_handle(), gui_font_image_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);

    VkImageMemoryBarrier usage_barrier {};
    usage_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    usage_barrier.pNext = nullptr;
    usage_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    usage_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    usage_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    usage_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    usage_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    usage_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    usage_barrier.image = gui_font_image_;
    usage_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    usage_barrier.subresourceRange.baseMipLevel = 0;
    usage_barrier.subresourceRange.levelCount = 1;
    usage_barrier.subresourceRange.baseArrayLayer = 0;
    usage_barrier.subresourceRange.layerCount = 1;
    graphics_cmd_buf_->vkCmdPipelineBarrier(
      VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
      0,
      0, nullptr,
      0, nullptr,
      1, &usage_barrier
    );
  }

  graphics_cmd_buf_->vkEndCommandBuffer();

  VkCommandBuffer cmd_buf = graphics_cmd_buf_->command_buffer();
  VkSubmitInfo submit_info {};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.pNext = nullptr;
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &cmd_buf;
  submit_info.waitSemaphoreCount = 0;
  submit_info.pWaitDstStageMask = nullptr;
  submit_info.pWaitSemaphores = nullptr;
  submit_info.signalSemaphoreCount = 0;
  submit_info.pSignalSemaphores = nullptr;
  graphics_queue_->vkQueueSubmit(1, &submit_info, VK_NULL_HANDLE);
  device_->vkDeviceWaitIdle();

  upload_buffer->destroy(*device_);

  // Save identifier so that it's returned to us by imgui later.
  // Not really necessary since we only have one font image but still.
  io.Fonts->TexID = reinterpret_cast<void*>(gui_font_image_);
}

void Renderer::update_gui_vertex_data(ImDrawData* draw_data) {
  // (Re)Create vertex buffer if needed
  size_t vertex_data_size = draw_data->TotalVtxCount * sizeof(ImDrawVert);
  if (gui_vertex_buffer_size_ < vertex_data_size) {
    if (gui_vertex_buffer_) {
      gui_vertex_buffer_->destroy(*device_);
    }
    gui_vertex_buffer_.reset(new vk::Buffer { *device_, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, vertex_data_size });
    gui_vertex_buffer_size_ = vertex_data_size;
  }

  // (Re)Create index buffer if needed
  size_t index_data_size = draw_data->TotalIdxCount * sizeof(ImDrawIdx);
  if (gui_index_buffer_size_ < index_data_size) {
    if (gui_index_buffer_) {
      gui_index_buffer_->destroy(*device_);
    }
    gui_index_buffer_.reset(new vk::Buffer { *device_, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, index_data_size });
    gui_index_buffer_size_ = index_data_size;
  }

  // Upload data to buffers
  {
    ImDrawVert* mapped_vertices { nullptr };
    ImDrawIdx* mapped_indices { nullptr };
    device_->vkMapMemory(gui_vertex_buffer_->vulkan_memory_handle(), 0, vertex_data_size, 0, reinterpret_cast<void**>(&mapped_vertices));
    device_->vkMapMemory(gui_index_buffer_->vulkan_memory_handle(), 0, index_data_size, 0, reinterpret_cast<void**>(&mapped_indices));

    for (int n = 0; n < draw_data->CmdListsCount; ++n) {
      ImDrawList const* cmd_list = draw_data->CmdLists[n];
      memcpy(mapped_vertices, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
      memcpy(mapped_indices, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
      mapped_vertices += cmd_list->VtxBuffer.Size;
      mapped_indices += cmd_list->IdxBuffer.Size;
    }

    VkMappedMemoryRange flush_ranges[2] = {};
    flush_ranges[0].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    flush_ranges[0].pNext = nullptr;
    flush_ranges[0].memory = gui_vertex_buffer_->vulkan_memory_handle();
    flush_ranges[0].offset = 0;
    flush_ranges[0].size = VK_WHOLE_SIZE;
    flush_ranges[1].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    flush_ranges[1].pNext = nullptr;
    flush_ranges[1].memory = gui_index_buffer_->vulkan_memory_handle();
    flush_ranges[1].offset = 0;
    flush_ranges[1].size = VK_WHOLE_SIZE;
    device_->vkFlushMappedMemoryRanges(2, flush_ranges);

    device_->vkUnmapMemory(gui_vertex_buffer_->vulkan_memory_handle());
    device_->vkUnmapMemory(gui_index_buffer_->vulkan_memory_handle());
  }
}

void Renderer::create_dgc_resources() {
  VkDeviceGeneratedCommandsFeaturesNVX features {};
  features.sType = VK_STRUCTURE_TYPE_DEVICE_GENERATED_COMMANDS_FEATURES_NVX;

  VkDeviceGeneratedCommandsLimitsNVX limits {};
  limits.sType = VK_STRUCTURE_TYPE_DEVICE_GENERATED_COMMANDS_LIMITS_NVX;

  device_->physical_device()->vkGetPhysicalDeviceGeneratedCommandsPropertiesNVX(&features, &limits);

  create_object_table();
  register_objects_in_table();
  create_indirect_commands_layout();
  reserve_space_for_indirect_commands();

  dgc_pipeline_parameters_.reset(new vk::Buffer { *device_, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, g_draw_call_count * sizeof(uint32_t) });

  // Initialize pipeline references to 0 (which means all would use the same
  // pipeline that was registered to index 0 by me).
  device_->vkMapMemory(dgc_pipeline_parameters_->vulkan_memory_handle(), 0, g_draw_call_count * sizeof(uint32_t), 0, reinterpret_cast<void**>(&mapped_dgc_pipeline_parameters_));
  memset(mapped_dgc_pipeline_parameters_, 0, g_draw_call_count * sizeof(uint32_t));
  VkMappedMemoryRange flush_range {};
  flush_range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
  flush_range.pNext = nullptr;
  flush_range.memory = dgc_pipeline_parameters_->vulkan_memory_handle();
  flush_range.offset = 0;
  flush_range.size = g_draw_call_count * sizeof(uint32_t);
  device_->vkFlushMappedMemoryRanges(1, &flush_range);

  // Size: one u32 for indexing the push constant entry in table, another one
  // for the actual push constant data (offset and size is set in the token).
  dgc_push_constants_.reset(new vk::Buffer { *device_, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, g_draw_call_count * sizeof(Push) });
  device_->vkMapMemory(dgc_push_constants_->vulkan_memory_handle(), 0, g_draw_call_count * sizeof(Push), 0, reinterpret_cast<void**>(&mapped_dgc_push_constants_));
}

void Renderer::create_object_table() {
  // Map objects in a table for use by the device when generating commands.
  // This is used because we can't use the CPU side pointers directly on the
  // device. Detailed resource bindings are registered later with
  // vkRegisterObjectsNVX.
  std::array<VkObjectEntryTypeNVX, 2> entry_types {
    VK_OBJECT_ENTRY_TYPE_PIPELINE_NVX,
    VK_OBJECT_ENTRY_TYPE_PUSH_CONSTANT_NVX,
  };

  std::array<uint32_t, 2> entry_counts {
    2,
    1,
  };

  std::array<VkObjectEntryUsageFlagsNVX, 2> entry_usage_flags {
    VK_OBJECT_ENTRY_USAGE_GRAPHICS_BIT_NVX,
    VK_OBJECT_ENTRY_USAGE_GRAPHICS_BIT_NVX,
  };

  static_assert(entry_types.size() == entry_counts.size() && entry_types.size() == entry_usage_flags.size());

  VkObjectTableCreateInfoNVX table_info {};
  table_info.sType = VK_STRUCTURE_TYPE_OBJECT_TABLE_CREATE_INFO_NVX;
  table_info.pNext = nullptr;
  table_info.objectCount = entry_types.size();
  table_info.pObjectEntryTypes = entry_types.data();
  table_info.pObjectEntryCounts = entry_counts.data();
  table_info.pObjectEntryUsageFlags = entry_usage_flags.data();
  table_info.maxUniformBuffersPerDescriptor = 0;
  table_info.maxStorageBuffersPerDescriptor = 1;
  table_info.maxStorageImagesPerDescriptor = 0;
  table_info.maxSampledImagesPerDescriptor = 0;
  table_info.maxPipelineLayouts = 1;

  VkResult result = device_->vkCreateObjectTableNVX(&table_info, nullptr, &object_table_);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Could not create object table.");
  }
}

void Renderer::register_objects_in_table() {
  // Note: resource bindings can be registered at arbitrary indices within a
  // table.
  VkObjectTablePipelineEntryNVX alpha_pipeline_entry {};
  alpha_pipeline_entry.type = VK_OBJECT_ENTRY_TYPE_PIPELINE_NVX;
  alpha_pipeline_entry.flags = VK_OBJECT_ENTRY_USAGE_GRAPHICS_BIT_NVX;
  alpha_pipeline_entry.pipeline = pipeline_dgc_solid_;

  VkObjectTablePipelineEntryNVX beta_pipeline_entry {};
  beta_pipeline_entry.type = VK_OBJECT_ENTRY_TYPE_PIPELINE_NVX;
  beta_pipeline_entry.flags = VK_OBJECT_ENTRY_USAGE_GRAPHICS_BIT_NVX;
  beta_pipeline_entry.pipeline = pipeline_dgc_wireframe_;

  VkObjectTablePushConstantEntryNVX push_constant_entry {};
  push_constant_entry.type = VK_OBJECT_ENTRY_TYPE_PUSH_CONSTANT_NVX;
  push_constant_entry.flags = VK_OBJECT_ENTRY_USAGE_GRAPHICS_BIT_NVX;
  push_constant_entry.pipelineLayout = gbuffer_pipeline_layout_;
  push_constant_entry.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

  std::array<VkObjectTableEntryNVX const*, 3> table_entries {
    reinterpret_cast<VkObjectTableEntryNVX*>(&alpha_pipeline_entry),
    reinterpret_cast<VkObjectTableEntryNVX*>(&beta_pipeline_entry),
    reinterpret_cast<VkObjectTableEntryNVX*>(&push_constant_entry),
  };

  std::array<uint32_t, 3> object_indices {
    0,
    1,
    0,
  };

  static_assert(table_entries.size() == object_indices.size());

  VkResult result = device_->vkRegisterObjectsNVX(object_table_, table_entries.size(), table_entries.data(), object_indices.data());
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Could not register objects into table.");
  }
}

void Renderer::create_indirect_commands_layout() {
  // Generating commands on the device is done by processing a bunch of token
  // sequences. It seems that a sequence is a set of commands to call after
  // each other, represented by tokens. That is, a draw call always comes
  // together with a pipeline switch according to the sequence defined here.
  // One can probably include several draw calls in a sequence if need be, but
  // it seems that a sequence is the atomic set of commands that will be used,
  // but with varying input data provided by buffers at generation time. The
  // VkIndirectCommandsLayoutNVX describes what calls a sequence consists of,
  // and we generate several invocations of this sequence later.

  std::array<VkIndirectCommandsLayoutTokenNVX, 3> tokens {};

  tokens[0].tokenType = VK_INDIRECT_COMMANDS_TOKEN_TYPE_PIPELINE_NVX;
  tokens[0].bindingUnit = 0; // Not used for pipelines
  tokens[0].dynamicCount = 0; // Not used for pipelines
  tokens[0].divisor = 1;

  tokens[1].tokenType = VK_INDIRECT_COMMANDS_TOKEN_TYPE_PUSH_CONSTANT_NVX;
  tokens[1].bindingUnit = sizeof(DirectX::XMFLOAT4X4) * 2; // For push constants, this is the |offset| parameter
  tokens[1].dynamicCount = sizeof(uint32_t); // For push constants, this is the |size| parameter
  tokens[1].divisor = 1;

  tokens[2].tokenType = VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_NVX;
  tokens[2].bindingUnit = 0; // Not used for draw
  tokens[2].dynamicCount = 0; // Not used for draw
  tokens[2].divisor = 1;

  VkIndirectCommandsLayoutCreateInfoNVX layout_info {};
  layout_info.sType = VK_STRUCTURE_TYPE_INDIRECT_COMMANDS_LAYOUT_CREATE_INFO_NVX;
  layout_info.pNext = nullptr;
  layout_info.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  layout_info.flags = 0; // VK_INDIRECT_COMMANDS_LAYOUT_USAGE_UNORDERED_SEQUENCES_BIT_NVX
  layout_info.tokenCount = static_cast<uint32_t>(tokens.size());
  layout_info.pTokens = tokens.data();

  VkResult result = device_->vkCreateIndirectCommandsLayoutNVX(&layout_info, nullptr, &indirect_commands_layout_);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Could not create indirect commands layout.");
  }
}

void Renderer::reserve_space_for_indirect_commands() {
  VkCommandBufferInheritanceInfo inheritance_info {};
  inheritance_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
  inheritance_info.pNext = nullptr;
  inheritance_info.renderPass = gbuffer_render_pass_;
  inheritance_info.subpass = 1;
  inheritance_info.framebuffer = framebuffer_;
  inheritance_info.occlusionQueryEnable = VK_FALSE;
  inheritance_info.queryFlags = 0;
  inheritance_info.pipelineStatistics = 0;

  VkCommandBufferBeginInfo cmd_buf_info {};
  cmd_buf_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  cmd_buf_info.pNext = nullptr;
  // Continue: Render passes (and subpasses) can't start in a secondary command
  // buffer, so the flag means that the command buffer continues using whatever
  // render pass is inherited from the primary command buffer. The reason we
  // don't always use it is because we can do non-render pass stuff in here as
  // well.
  // https://www.khronos.org/assets/uploads/developers/library/2016-vulkan-devday-uk/6-Vulkan-subpasses.pdf
  cmd_buf_info.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
  cmd_buf_info.pInheritanceInfo = &inheritance_info;

  dgc_cmd_buf_->vkBeginCommandBuffer(&cmd_buf_info);

  VkCmdReserveSpaceForCommandsInfoNVX reserve_info {};
  reserve_info.sType = VK_STRUCTURE_TYPE_CMD_RESERVE_SPACE_FOR_COMMANDS_INFO_NVX;
  reserve_info.pNext = nullptr;
  reserve_info.objectTable = object_table_;
  reserve_info.indirectCommandsLayout = indirect_commands_layout_;
  // If we want to save generated draw calls into a secondary command buffer we
  // need to allocate memory up-front. This call is done on the CPU, but memory
  // does indeed live on the GPU according to Christoph Kubisch. The number of
  // sequences is an upper limit on how many times the indirect commands layout
  // will be stamped out, I believe.
  reserve_info.maxSequencesCount = g_draw_call_count;

  // Must be called in a render pass...
  dgc_cmd_buf_->vkCmdReserveSpaceForCommandsNVX(&reserve_info);

  dgc_cmd_buf_->vkEndCommandBuffer();

  // Note: Execution is done the first time in my primary command buffer.
  // Why oh why does this have to be called in a render pass? It is already
  // a secondary command buffer, meaning I can't execute it by itself so I
  // already need a primary buffer to drive it... sigh
}

void Renderer::create_timing_resources() {
  VkQueryPoolCreateInfo pool_info {};
  pool_info.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
  pool_info.pNext = nullptr;
  pool_info.flags = 0;
  pool_info.queryType = VK_QUERY_TYPE_TIMESTAMP;
  pool_info.queryCount = 4;
  pool_info.pipelineStatistics = 0;

  VkResult result = device_->vkCreateQueryPool(&pool_info, nullptr, &query_pool_);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Could not create query pool.");
  }
}
