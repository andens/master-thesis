#pragma once

#include "vulkan_include.inl"

namespace vk {

class Device;

class UniformBuffer {
public:
  UniformBuffer(Device& device, VkMemoryPropertyFlagBits memory_properties, VkDeviceSize buffer_size);
  void destroy(Device& device);
  VkBuffer vulkan_buffer_handle() const { return buffer_; }
  VkDeviceMemory vulkan_memory_handle() const { return memory_; }

private:
  UniformBuffer(UniformBuffer& other) = delete;
  void operator=(UniformBuffer& rhs) = delete;

  void create_buffer(Device& device, VkDeviceSize buffer_size);
  void allocate_memory(Device& device, VkMemoryPropertyFlagBits desired_memory_properties);

private:
  VkBuffer buffer_ = VK_NULL_HANDLE;
  VkDeviceMemory memory_ = VK_NULL_HANDLE;
};

} // namespace vk
