#pragma once

#include <memory>
#include <vulkan-helpers/vulkan_include.inl>

namespace vkgen {
class GlobalFunctions;
}

namespace vk {
class Instance;
}

class Renderer {
public:
  Renderer();
  ~Renderer();

private:
  void create_instance();
  void create_debug_callback();

private:
  std::unique_ptr<vkgen::GlobalFunctions> vk_globals_;
  std::unique_ptr<vk::Instance> instance_;
  VkDebugReportCallbackEXT debug_callback_ { VK_NULL_HANDLE };
};
