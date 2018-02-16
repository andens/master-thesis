#pragma once

#include <array>
#include <vulkan-helpers/vulkan_include.inl>

namespace vk {
class Device;
}

namespace graphics {
namespace deferred_shading {

class GBuffer {
public:
  struct Buffer {
    VkImage image = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkFormat format = VK_FORMAT_UNDEFINED;
    VkMemoryRequirements mem_req = {};
    VkDeviceSize mem_offset = 0;
  };

  enum class BufferTypes {
    Albedo = 0,
    Normal,
    BufferCount,
  };

public:
  GBuffer(vk::Device* device, uint32_t width, uint32_t height);
  ~GBuffer();

  uint32_t width() const { return width_; }
  uint32_t height() const { return height_; }
  std::array<Buffer, (size_t)BufferTypes::BufferCount> const& buffers() const { return buffers_; }
  Buffer const& get_buf(BufferTypes which) const;

private:
  GBuffer(GBuffer& other) = delete;
  void operator=(GBuffer& rhs) = delete;

  void create_buffer_(VkFormat format, Buffer* buffer, uint32_t width, uint32_t height);
  void allocate_memory_(void);
  void bind_buffers_to_memory_(void);
  void create_image_views_(void);

private:
  vk::Device* device_ = nullptr;

  uint32_t width_ = 0;
  uint32_t height_ = 0;

  VkDeviceMemory memory_ = VK_NULL_HANDLE;
  std::array<Buffer, (size_t)BufferTypes::BufferCount> buffers_;
};

} // deferred_shading
} // graphics
