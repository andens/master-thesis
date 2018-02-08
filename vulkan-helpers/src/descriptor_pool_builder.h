#pragma once

#include <cstdint>
#include <vector>
#include "vulkan_include.inl"

namespace vk {

class Device;

class DescriptorPoolBuilder {
public:
  void reserve(VkDescriptorType descriptor_type, uint32_t count = 1);
  void max_sets(uint32_t sets);
  VkDescriptorPool build(Device& device);

private:
  std::vector<VkDescriptorPoolSize> pool_sizes_;
  VkDescriptorPoolCreateInfo pool_info_ = {};
};

}
