#include "descriptor_pool_builder.h"

#include "device.h"

namespace vk {

void DescriptorPoolBuilder::reserve(VkDescriptorType descriptor_type, uint32_t count) {
  VkDescriptorPoolSize size = {};
  size.type = descriptor_type;
  size.descriptorCount = count;
  pool_sizes_.push_back(size);
}

void DescriptorPoolBuilder::max_sets(uint32_t sets) {
  // Maximum number of descriptor sets that will be allocated
  pool_info_.maxSets = sets;
}

VkDescriptorPool DescriptorPoolBuilder::build(Device& device) {
  pool_info_.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  pool_info_.pNext = nullptr;
  // Optional flag that indicates if individual descriptor sets can be freed
  pool_info_.flags = 0;
  pool_info_.poolSizeCount = pool_sizes_.size();
  pool_info_.pPoolSizes = pool_sizes_.data();

  VkDescriptorPool pool = VK_NULL_HANDLE;
  VkResult result = device.vkCreateDescriptorPool(&pool_info_, nullptr, &pool);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Could not create descriptor pool.");
  }

  return pool;
}

}
