#pragma once

#include <memory>
#include <vulkan-helpers/vulkan_include.inl>

namespace vkgen {
class GlobalFunctions;
}

namespace vk {
class CommandBuffer;
class CommandPool;
class Device;
class Instance;
class PresentationSurface;
class Queue;
class Swapchain;
}

class Renderer {
public:
  Renderer(HWND hwnd);
  ~Renderer();

private:
  void create_instance();
  void create_debug_callback();
  void create_surface(HWND hwnd);
  void create_device();
  void create_swapchain();
  void create_command_pools_and_buffers();

private:
  std::unique_ptr<vkgen::GlobalFunctions> vk_globals_;
  std::shared_ptr<vk::Instance> instance_;
  VkDebugReportCallbackEXT debug_callback_ { VK_NULL_HANDLE };
  std::shared_ptr<vk::PresentationSurface> surface_;
  std::shared_ptr<vk::Device> device_;
  std::shared_ptr<vk::Queue> graphics_queue_;
  std::shared_ptr<vk::Queue> compute_queue_;
  std::shared_ptr<vk::Queue> present_queue_;
  std::unique_ptr<vk::Swapchain> swapchain_;
  std::shared_ptr<vk::CommandPool> stable_graphics_cmd_pool_;
  std::shared_ptr<vk::CommandPool> transient_graphics_cmd_pool_;
  std::shared_ptr<vk::CommandBuffer> graphics_cmd_buf_;
  std::shared_ptr<vk::CommandBuffer> blit_swapchain_cmd_buf_;
};
