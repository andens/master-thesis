#include "renderer.h"

#include <algorithm>
#include <iostream>
#include <vector>
#include <vulkan-helpers/device.h>
#include <vulkan-helpers/device_builder.h>
#include <vulkan-helpers/instance.h>
#include <vulkan-helpers/instance_builder.h>
#include <vulkan-helpers/presentation_surface.h>
#include <vulkan-helpers/vk_dispatch_tables.h>

Renderer::Renderer(HWND hwnd) {
  vk_globals_.reset(new vkgen::GlobalFunctions("vulkan-1.dll"));

  create_instance();
  create_debug_callback();
  create_surface(hwnd);
  create_device();
}

Renderer::~Renderer() {
  if (device_->device()) {
    device_->vkDeviceWaitIdle();
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
    std::cout << pMessage << std::endl;
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
