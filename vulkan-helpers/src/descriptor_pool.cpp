#include "descriptor_pool.h"

#include "descriptor_pool_builder.h"
#include "descriptor_set.h"
#include "descriptor_set_layout.h"
#include "device.h"

namespace vk {

DescriptorPool::DescriptorPool(Device& device, DescriptorPoolBuilder& builder) {
  pool_ = builder.build(device);
}

void DescriptorPool::destroy(Device& device) {
  if (pool_) {
    device.vkDestroyDescriptorPool(pool_, nullptr);
    pool_ = VK_NULL_HANDLE;
  }
}

std::unique_ptr<DescriptorSet> DescriptorPool::allocate(Device& device, DescriptorSetLayout& layout) {
  auto set_layout = layout.vulkan_handle();

  VkDescriptorSetAllocateInfo alloc_info = {};
  alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  alloc_info.pNext = nullptr;
  alloc_info.descriptorPool = pool_;
  alloc_info.descriptorSetCount = 1;
  alloc_info.pSetLayouts = &set_layout;

  VkDescriptorSet set = VK_NULL_HANDLE;
  VkResult result = device.vkAllocateDescriptorSets(&alloc_info, &set);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Could not allocate descriptor set.");
  }

  return std::unique_ptr<DescriptorSet>(new DescriptorSet(set));
}

}
