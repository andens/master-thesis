#include "renderer.h"

#include <algorithm>
#include <array>
#include <fstream>
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
#include "../mesh/mesh.h"
#include "../obj-loader/obj-loader.h"
#include "../render-jobs-descriptor-set/render-jobs-descriptor-set.h"

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
  create_descriptor_sets();
  create_render_pass();
  create_framebuffer();
  create_shaders();
  create_pipeline();
  create_synchronization_primitives();
  configure_barrier_structs();
  create_vertex_buffer();
  create_indirect_buffer();

  DirectX::XMStoreFloat4x4(&view_, DirectX::XMMatrixIdentity());
  DirectX::XMStoreFloat4x4(&proj_, DirectX::XMMatrixIdentity());
}

Renderer::~Renderer() {
  if (device_->device()) {
    device_->vkDeviceWaitIdle();
    indirect_buffer_->destroy(*device_);
    vertex_buffer_->destroy(*device_);
    device_->vkDestroyFence(gbuffer_generation_fence_, nullptr);
    device_->vkDestroyFence(render_fence_, nullptr);
    device_->vkDestroySemaphore(gbuffer_generation_complete_, nullptr);
    device_->vkDestroySemaphore(blit_swapchain_complete_, nullptr);
    device_->vkDestroySemaphore(image_available_semaphore_, nullptr);
    device_->vkDestroyPipeline(gbuffer_pipeline_, nullptr);
    device_->vkDestroyPipelineLayout(gbuffer_pipeline_layout_, nullptr);
    device_->vkDestroyShaderModule(fullscreen_triangle_vs_, nullptr);
    device_->vkDestroyShaderModule(fill_gbuffer_vs_, nullptr);
    device_->vkDestroyShaderModule(fill_gbuffer_fs_, nullptr);
    device_->vkDestroyFramebuffer(framebuffer_, nullptr);
    device_->vkDestroyRenderPass(gbuffer_render_pass_, nullptr);
    render_jobs_descriptor_set_->destroy(*device_);
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

  VkCommandBufferBeginInfo begin_info {};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  begin_info.pNext = nullptr;
  begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  begin_info.pInheritanceInfo = nullptr;

  graphics_cmd_buf_->vkBeginCommandBuffer(&begin_info);

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

  graphics_cmd_buf_->vkCmdBindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, gbuffer_pipeline_);

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
  //graphics_cmd_buf_->vkCmdDraw(36, 1, 0, 0);
  //graphics_cmd_buf_->vkCmdDraw(2160, 1, 36, 1);
  graphics_cmd_buf_->vkCmdDrawIndirect(indirect_buffer_->vulkan_buffer_handle(), 0, 2, sizeof(VkDrawIndirectCommand));

  graphics_cmd_buf_->vkCmdEndRenderPass();

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

  builder.use_layer("VK_LAYER_LUNARG_standard_validation");

  builder.use_extension("VK_EXT_debug_report");
  builder.use_extension("VK_KHR_surface");
#if defined(_WIN32)
  builder.use_extension("VK_KHR_win32_surface");
#else
#error "Unsupported OS"
#endif

  instance_ = builder.build(vk_globals_.get());
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
  device_ = builder.build(instance_.get());
  graphics_queue_ = device_->graphics_queue();
  compute_queue_ = device_->compute_queue();
  present_queue_ = device_->present_queue();
}

void Renderer::create_swapchain() {
  swapchain_.reset(new vk::Swapchain(device_, surface_));
}

void Renderer::create_command_pools_and_buffers() {
  stable_graphics_cmd_pool_ = vk::CommandPool::make_stable(device_->graphics_family(), device_);
  graphics_cmd_buf_ = stable_graphics_cmd_pool_->allocate_primary();

  transient_graphics_cmd_pool_ = vk::CommandPool::make_transient(device_->graphics_family(), device_);
  blit_swapchain_cmd_buf_ = transient_graphics_cmd_pool_->allocate_primary();
}

void Renderer::create_render_pass() {
  const uint32_t buf_count = static_cast<uint32_t>(graphics::deferred_shading::GBuffer::BufferTypes::BufferCount);

  vk::RenderPassBuilder builder;
  for (uint32_t i = 0; i < buf_count; ++i) {
    builder.attachment(vk::RenderPassAttachment::c_clear_store(gbuffer_->buffers()[i].format));
  }
  builder.attachment(vk::RenderPassAttachment::d_clear_store(VK_FORMAT_D32_SFLOAT));
  builder.graphics_subpass([buf_count](vk::Subpass& subpass) {
    for (uint32_t i = 0; i < buf_count; ++i) {
      subpass.color_attachment(i, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    }
    subpass.depth_stencil_attachment(buf_count, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
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
  dependency.dstSubpass = VK_SUBPASS_EXTERNAL;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
  dependency.dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
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
}

void Renderer::create_pipeline() {
  vk::PipelineLayoutBuilder layout_builder;
  layout_builder.push_constant(VK_SHADER_STAGE_VERTEX_BIT, 0, 2 * sizeof(DirectX::XMFLOAT4X4));
  layout_builder.descriptor_layout(render_jobs_descriptor_set_->layout().vulkan_handle());
  gbuffer_pipeline_layout_ = layout_builder.build(*device_);

  vk::PipelineBuilder pipeline_builder;

  pipeline_builder.shader_stage(VK_SHADER_STAGE_VERTEX_BIT, fill_gbuffer_vs_);
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

  gbuffer_pipeline_ = pipeline_builder.build(*device_, gbuffer_pipeline_layout_, gbuffer_render_pass_);
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
  vertices.reserve(vertices.size() + 36);
  vertices.insert(vertices.end(), {
    // Front
    { { -0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f }, { 0.0f, 0.0f, -1.0f } },
    { { -0.5f, +0.5f, -0.5f }, { 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } },
    { { +0.5f, +0.5f, -0.5f }, { 1.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } },
    { { +0.5f, +0.5f, -0.5f }, { 1.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } },
    { { +0.5f, -0.5f, -0.5f }, { 1.0f, 1.0f }, { 0.0f, 0.0f, -1.0f } },
    { { -0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f }, { 0.0f, 0.0f, -1.0f } },
    // Right
    { { +0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f } },
    { { +0.5f, +0.5f, -0.5f }, { 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
    { { +0.5f, +0.5f, +0.5f }, { 1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
    { { +0.5f, +0.5f, +0.5f }, { 1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
    { { +0.5f, -0.5f, +0.5f }, { 1.0f, 1.0f }, { 1.0f, 0.0f, 0.0f } },
    { { +0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f } },
    // Back
    { { +0.5f, -0.5f, +0.5f }, { 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f } },
    { { +0.5f, +0.5f, +0.5f }, { 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
    { { -0.5f, +0.5f, +0.5f }, { 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
    { { -0.5f, +0.5f, +0.5f }, { 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
    { { -0.5f, -0.5f, +0.5f }, { 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f } },
    { { +0.5f, -0.5f, +0.5f }, { 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f } },
    // Left
    { { -0.5f, -0.5f, +0.5f }, { 0.0f, 1.0f }, { -1.0f, 0.0f, 0.0f } },
    { { -0.5f, +0.5f, +0.5f }, { 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f } },
    { { -0.5f, +0.5f, -0.5f }, { 1.0f, 0.0f }, { -1.0f, 0.0f, 0.0f } },
    { { -0.5f, +0.5f, -0.5f }, { 1.0f, 0.0f }, { -1.0f, 0.0f, 0.0f } },
    { { -0.5f, -0.5f, -0.5f }, { 1.0f, 1.0f }, { -1.0f, 0.0f, 0.0f } },
    { { -0.5f, -0.5f, +0.5f }, { 0.0f, 1.0f }, { -1.0f, 0.0f, 0.0f } },
    // Top
    { { -0.5f, +0.5f, -0.5f }, { 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } },
    { { -0.5f, +0.5f, +0.5f }, { 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
    { { +0.5f, +0.5f, +0.5f }, { 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
    { { +0.5f, +0.5f, +0.5f }, { 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
    { { +0.5f, +0.5f, -0.5f }, { 1.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } },
    { { -0.5f, +0.5f, -0.5f }, { 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } },
    // Bottom
    { { -0.5f, -0.5f, +0.5f }, { 0.0f, 1.0f }, { 0.0f, -1.0f, 0.0f } },
    { { -0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } },
    { { +0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } },
    { { +0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } },
    { { +0.5f, -0.5f, +0.5f }, { 1.0f, 1.0f }, { 0.0f, -1.0f, 0.0f } },
    { { -0.5f, -0.5f, +0.5f }, { 0.0f, 1.0f }, { 0.0f, -1.0f, 0.0f } },
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
  builder.max_sets(1);
  descriptor_pool_.reset(new vk::DescriptorPool { *device_, builder });

  render_jobs_descriptor_set_.reset(new RenderJobsDescriptorSet { *device_, *descriptor_pool_ });

  // TODO: Hardcoded render job for now
  render_jobs_descriptor_set_->update_data(*device_, 0, [](RenderJobsDescriptorSet::RenderJobData& data) {
    DirectX::XMStoreFloat4x4(&data.transform, DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f));
  });
  render_jobs_descriptor_set_->update_data(*device_, 1, [](RenderJobsDescriptorSet::RenderJobData& data) {
    DirectX::XMStoreFloat4x4(&data.transform, DirectX::XMMatrixTranslation(5.0f, 0.0f, 0.0f));
  });
}

void Renderer::create_indirect_buffer() {
  indirect_buffer_.reset(new vk::Buffer { *device_, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, sizeof(VkDrawIndirectCommand) * max_draw_calls_ });

  // TODO: Hardcoded mapping for. This is to be replaced with proper draw call management
  VkDrawIndirectCommand* mapped_data = nullptr;
  device_->vkMapMemory(indirect_buffer_->vulkan_memory_handle(), 0, sizeof(VkDrawIndirectCommand) * 2, 0, reinterpret_cast<void**>(&mapped_data));

  mapped_data->vertexCount = 36;
  mapped_data->instanceCount = 1;
  mapped_data->firstVertex = 0;
  mapped_data->firstInstance = 0;
  ++mapped_data;
  mapped_data->vertexCount = 2160;
  mapped_data->instanceCount = 1;
  mapped_data->firstVertex = 36;
  mapped_data->firstInstance = 0;

  VkMappedMemoryRange flush_range {};
  flush_range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
  flush_range.pNext = nullptr;
  flush_range.memory = indirect_buffer_->vulkan_memory_handle();
  flush_range.offset = 0;
  flush_range.size = sizeof(VkDrawIndirectCommand) * 2;
  device_->vkFlushMappedMemoryRanges(1, &flush_range);

  device_->vkUnmapMemory(indirect_buffer_->vulkan_memory_handle());
}
