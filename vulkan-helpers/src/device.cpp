#include "device.h"

namespace vk {

VkSemaphore Device::create_semaphore() {
  VkSemaphoreCreateInfo semaphore_info = {};
  semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  semaphore_info.pNext = nullptr;
  semaphore_info.flags = 0;

  VkSemaphore semaphore = VK_NULL_HANDLE;
  VkResult result = vkCreateSemaphore(&semaphore_info, nullptr, &semaphore);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Could not create semaphore.");
  }

  return semaphore;
}

VkFence Device::create_fence(bool signaled) {
  VkFenceCreateInfo fence_info = {};
  fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fence_info.pNext = nullptr;
  fence_info.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

  VkFence fence = VK_NULL_HANDLE;
  VkResult result = vkCreateFence(&fence_info, nullptr, &fence);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Could not create fence.");
  }

  return fence;
}

}
