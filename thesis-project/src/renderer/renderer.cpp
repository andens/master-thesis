#include "renderer.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <vector>
#include <vulkan-helpers/command_buffer.h>
#include <vulkan-helpers/command_pool.h>
#include <vulkan-helpers/device.h>
#include <vulkan-helpers/device_builder.h>
#include <vulkan-helpers/instance.h>
#include <vulkan-helpers/instance_builder.h>
#include <vulkan-helpers/pipeline_builder.h>
#include <vulkan-helpers/pipeline_layout_builder.h>
#include <vulkan-helpers/pipeline_vertex_layout.h>
#include <vulkan-helpers/presentation_surface.h>
#include <vulkan-helpers/render_pass_builder.h>
#include <vulkan-helpers/swapchain.h>
#include <vulkan-helpers/vk_dispatch_tables.h>
#include "../depth-buffer/depth_buffer.h"
#include "../gbuffer/gbuffer.h"

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
  create_render_pass();
  create_framebuffer();
  create_shaders();
  create_pipeline();
  create_synchronization_primitives();
}

Renderer::~Renderer() {
  if (device_->device()) {
    device_->vkDeviceWaitIdle();
    device_->vkDestroyFence(render_fence_, nullptr);
    device_->vkDestroySemaphore(blit_swapchain_complete_, nullptr);
    device_->vkDestroySemaphore(image_available_semaphore_, nullptr);
    device_->vkDestroyPipeline(gbuffer_pipeline_, nullptr);
    device_->vkDestroyPipelineLayout(gbuffer_pipeline_layout_, nullptr);
    device_->vkDestroyShaderModule(fullscreen_triangle_vs_, nullptr);
    device_->vkDestroyShaderModule(fill_gbuffer_vs_, nullptr);
    device_->vkDestroyShaderModule(fill_gbuffer_fs_, nullptr);
    device_->vkDestroyFramebuffer(framebuffer_, nullptr);
    device_->vkDestroyRenderPass(gbuffer_render_pass_, nullptr);
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
  // TODO: No descriptor set or push constants for now.
  gbuffer_pipeline_layout_ = layout_builder.build(*device_);

  vk::PipelineBuilder pipeline_builder;

  pipeline_builder.shader_stage(VK_SHADER_STAGE_VERTEX_BIT, fill_gbuffer_vs_);
  pipeline_builder.shader_stage(VK_SHADER_STAGE_FRAGMENT_BIT, fill_gbuffer_fs_);

  pipeline_builder.vertex_layout([](auto layout) {
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
  render_fence_ = device_->create_fence(true);
}
