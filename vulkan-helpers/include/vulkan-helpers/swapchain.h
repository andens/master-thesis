#pragma once

#include <memory>
#include <vector>
#include "vulkan_include.inl"

namespace vk {

class Device;
class PresentationSurface;

class Swapchain {
public:
  Swapchain(std::weak_ptr<Device> device,
      std::weak_ptr<PresentationSurface> surface);
  ~Swapchain();

  VkSwapchainKHR get_swapchain() const { return swapchain_; }
  VkImage image(uint32_t index) const;
  void resize();

private:
  Swapchain(Swapchain& other) = delete;
  void operator=(Swapchain& rhs) = delete;

private:
  std::weak_ptr<Device> device_;

  VkSwapchainKHR swapchain_ = VK_NULL_HANDLE;
  VkFormat format_ = VK_FORMAT_UNDEFINED;
  std::vector<VkImage> images_;
  std::vector<VkImageView> image_views_;
  std::weak_ptr<PresentationSurface> surface_;
};

}
