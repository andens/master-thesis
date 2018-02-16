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
#include <vulkan-helpers/presentation_surface.h>
#include <vulkan-helpers/render_pass_builder.h>
#include <vulkan-helpers/swapchain.h>
#include <vulkan-helpers/vk_dispatch_tables.h>

Renderer::Renderer(HWND hwnd) {
  vk_globals_.reset(new vkgen::GlobalFunctions("vulkan-1.dll"));

  create_instance();
  create_debug_callback();
  create_surface(hwnd);
  create_device();
  create_swapchain();
  create_command_pools_and_buffers();
  create_render_pass();
  create_shaders();
}

Renderer::~Renderer() {
  if (device_->device()) {
    device_->vkDeviceWaitIdle();
    device_->vkDestroyShaderModule(fullscreen_triangle_vs_, nullptr);
    device_->vkDestroyShaderModule(fill_gbuffer_vs_, nullptr);
    device_->vkDestroyShaderModule(fill_gbuffer_fs_, nullptr);
    device_->vkDestroyRenderPass(gbuffer_render_pass_, nullptr);
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
  vk::RenderPassBuilder builder;
  builder.attachment(vk::RenderPassAttachment::c_clear_store(VK_FORMAT_R8G8B8A8_UNORM));
  builder.attachment(vk::RenderPassAttachment::d_clear_store(VK_FORMAT_D24_UNORM_S8_UINT));
  builder.graphics_subpass([](vk::Subpass& subpass) {
    subpass.color_attachment(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    subpass.depth_stencil_attachment(1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
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
