#pragma once

#include <cstdint>
#include <vulkan-helpers/vulkan_include.inl>

namespace vk {
class CommandBuffer;
class Device;
class Queue;
}

namespace graphics {

class DepthBuffer {
public:
  DepthBuffer(vk::Device* device, uint32_t width, uint32_t height, VkFormat depth_format, vk::CommandBuffer& gfx_cmd_buf, vk::Queue& gfx_queue);
  ~DepthBuffer();

  VkFormat get_format(void) const { return format_; }
  VkImage get_buffer(void) const { return buffer_image_; }
  VkImageView get_view(void) const { return buffer_image_view_; }

private:
  DepthBuffer(DepthBuffer& other) = delete;
  void operator=(DepthBuffer& rhs) = delete;

  void create_image(void);
  void allocate_image_memory(void);
  void create_view(void);
  void change_image_layout(vk::CommandBuffer& gfx_cmd_buf, vk::Queue& gfx_queue);

private:
  vk::Device* device_ = nullptr;
  uint32_t width_ = 0;
  uint32_t height_ = 0;
  VkFormat format_ = VK_FORMAT_UNDEFINED;
  VkImage buffer_image_ = VK_NULL_HANDLE;
  VkDeviceMemory buffer_image_memory_ = VK_NULL_HANDLE;
  VkImageView buffer_image_view_ = VK_NULL_HANDLE;
};

}
