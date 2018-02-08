#pragma once

#include <vector>
#include "vulkan_include.inl"

namespace vk {

class Device;

class DescriptorSet {
public:
  DescriptorSet(VkDescriptorSet set);
  VkDescriptorSet vulkan_handle() const { return set_; }
  void write_storage_buffer(uint32_t binding, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range);
  void write_storage_texel_buffer(uint32_t binding, VkBufferView view);
  void write_uniform_buffer(uint32_t binding, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range);
  void update_pending(vk::Device& device);

private:
  VkDescriptorSet set_ = VK_NULL_HANDLE;
  std::vector<VkWriteDescriptorSet> descriptor_writes_;
  std::vector<VkDescriptorBufferInfo> buffer_descriptors_;
  std::vector<VkBufferView> texel_buffer_views_;
};

}
