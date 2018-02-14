#pragma once

#include <memory>
#include <vulkan-helpers/vulkan_include.inl>

namespace vkgen {
class GlobalFunctions;
}

namespace vk {
class Device;
class Instance;
class PresentationSurface;
class Queue;
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

private:
  std::unique_ptr<vkgen::GlobalFunctions> vk_globals_;
  std::shared_ptr<vk::Instance> instance_;
  VkDebugReportCallbackEXT debug_callback_ { VK_NULL_HANDLE };
  std::unique_ptr<vk::PresentationSurface> surface_;
  std::unique_ptr<vk::Device> device_;
  std::shared_ptr<vk::Queue> graphics_queue_;
  std::shared_ptr<vk::Queue> compute_queue_;
  std::shared_ptr<vk::Queue> present_queue_;
};
