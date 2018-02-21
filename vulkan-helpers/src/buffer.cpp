#include "buffer.h"

#include "device.h"
#include "physical_device.h"

namespace vk {

void Buffer::destroy(Device& device) {
  if (buffer_) {
    device.vkDestroyBuffer(buffer_, nullptr);
    buffer_ = VK_NULL_HANDLE;
  }

  if (memory_) {
    device.vkFreeMemory(memory_, nullptr);
    memory_ = VK_NULL_HANDLE;
  }
}

Buffer::Buffer(Device& device, VkBufferUsageFlags usage, VkMemoryPropertyFlagBits memory_properties, VkDeviceSize buffer_size) {
  VkBufferCreateInfo buffer_info = {};
  buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  buffer_info.pNext = nullptr;
  buffer_info.flags = 0;
  buffer_info.size = buffer_size;
  buffer_info.usage = usage;
  buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  buffer_info.queueFamilyIndexCount = 0;
  buffer_info.pQueueFamilyIndices = nullptr;

  VkResult result = device.vkCreateBuffer(&buffer_info, nullptr, &buffer_);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Could not create buffer.");
  }

  allocate_memory(device, memory_properties);

  result = device.vkBindBufferMemory(buffer_, memory_, 0);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Could not bind buffer to memory.");
  }
}

void Buffer::allocate_memory(Device& device, VkMemoryPropertyFlagBits desired_memory_properties) {
  VkMemoryRequirements mem_req;
  device.vkGetBufferMemoryRequirements(buffer_, &mem_req);

  auto& memory_properties = device.physical_device()->memory_properties();

  for (uint32_t i = 0; i < memory_properties.memoryTypeCount; ++i) {
    // Current memory type supports the buffer and we can map it.
    if ((mem_req.memoryTypeBits & (1 << i)) && ((memory_properties.memoryTypes[i].propertyFlags & desired_memory_properties) == desired_memory_properties)) {
      VkMemoryAllocateInfo memory_allocate_info = {};
      memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
      memory_allocate_info.pNext = nullptr;
      memory_allocate_info.allocationSize = mem_req.size;
      memory_allocate_info.memoryTypeIndex = i;

      VkResult result = device.vkAllocateMemory(&memory_allocate_info, nullptr, &memory_);
      if (result == VK_SUCCESS) {
        return;
      }
    }
  }

  throw std::runtime_error("Could not allocate memory for uniform buffer.");
}

} // namespace vk
