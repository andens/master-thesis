#include "command_pool.h"

#include "device.h"
#include "command_buffer.h"

using namespace std;

namespace vk {

unique_ptr<CommandPool> CommandPool::make_stable(uint32_t queue_family, weak_ptr<Device> device) {
  VkCommandPoolCreateInfo pool_info = {};
  pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  pool_info.pNext = nullptr;
  pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  pool_info.queueFamilyIndex = queue_family;

  return make(pool_info, device);
}

unique_ptr<CommandPool> CommandPool::make_transient(uint32_t queue_family, weak_ptr<Device> device) {
  VkCommandPoolCreateInfo pool_info = {};
  pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  pool_info.pNext = nullptr;
  // Reset allows command buffers to be reset individually as opposed to all at
  // once. Transient is a hint that command buffers are recorded often.
  pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
  pool_info.queueFamilyIndex = queue_family;

  return make(pool_info, device);
}

unique_ptr<CommandBuffer> CommandPool::allocate_primary() {
  VkCommandBufferAllocateInfo alloc_info = {};
  alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  alloc_info.pNext = nullptr;
  alloc_info.commandPool = pool_;
  alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  alloc_info.commandBufferCount = 1;

  VkCommandBuffer buf = VK_NULL_HANDLE;
  VkResult result = device_.lock()->vkAllocateCommandBuffers(&alloc_info, &buf);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to allocate command buffer.");
  }

  return unique_ptr<CommandBuffer>(new CommandBuffer(buf, device_));
}

unique_ptr<CommandBuffer> CommandPool::allocate_secondary() {
  VkCommandBufferAllocateInfo alloc_info = {};
  alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  alloc_info.pNext = nullptr;
  alloc_info.commandPool = pool_;
  alloc_info.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
  alloc_info.commandBufferCount = 1;

  VkCommandBuffer buf = VK_NULL_HANDLE;
  VkResult result = device_.lock()->vkAllocateCommandBuffers(&alloc_info, &buf);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to allocate command buffer.");
  }

  return unique_ptr<CommandBuffer>(new CommandBuffer(buf, device_));
}

CommandPool::~CommandPool() {
  if (pool_) {
    device_.lock()->vkDestroyCommandPool(pool_, nullptr);
  }
}

unique_ptr<CommandPool> CommandPool::make(VkCommandPoolCreateInfo const& pool_info, weak_ptr<Device> device) {
  auto dev = device.lock();

  VkCommandPool pool = VK_NULL_HANDLE;
  VkResult result = dev->vkCreateCommandPool(&pool_info, nullptr, &pool);
  if (result != VK_SUCCESS) {
    throw runtime_error("Failed to create command pool.");
  }

  auto ret = unique_ptr<CommandPool>(new CommandPool);
  ret->pool_ = pool;
  ret->device_ = device;
  return ret;
}

}
