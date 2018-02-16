#include "gbuffer.h"

#include <algorithm>
#include <vulkan-helpers/device.h>
#include <vulkan-helpers/physical_device.h>

using namespace std;

namespace graphics {
namespace deferred_shading {

GBuffer::GBuffer(vk::Device* device, uint32_t width, uint32_t height)
  : device_(device), width_(width), height_(height) {
  create_buffer_(VK_FORMAT_R8G8B8A8_UNORM, &buffers_[static_cast<uint32_t>(BufferTypes::Albedo)], width, height);
  create_buffer_(VK_FORMAT_R8G8B8A8_UNORM, &buffers_[static_cast<uint32_t>(BufferTypes::Normal)], width, height);

  allocate_memory_();
  bind_buffers_to_memory_();
  create_image_views_();
}

GBuffer::~GBuffer() {
  for (auto& buf : buffers_) {
    if (buf.view) {
      device_->vkDestroyImageView(buf.view, nullptr);
      buf.view = VK_NULL_HANDLE;
    }

    if (buf.image) {
      device_->vkDestroyImage(buf.image, nullptr);
      buf.image = VK_NULL_HANDLE;
    }
  }

  if (memory_) {
    device_->vkFreeMemory(memory_, nullptr);
    memory_ = VK_NULL_HANDLE;
  }
}

GBuffer::Buffer const& GBuffer::get_buf(GBuffer::BufferTypes which) const {
  if (which >= BufferTypes::BufferCount) {
    throw std::runtime_error("GBuffer::get_buf: invalid buffer type.");
  }

  return buffers_[static_cast<uint32_t>(which)];
}

void GBuffer::create_buffer_(VkFormat format, Buffer* buffer, uint32_t width, uint32_t height) {
  buffer->format = format;

  // Note: according to vkGetPhysicalDeviceFormatProperties, load and store
  // with storage usage flag is supported in compute shaders for image formats
  // with VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT.
  VkImageCreateInfo image_info = {};
  image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  image_info.pNext = nullptr;
  image_info.flags = 0;
  image_info.imageType = VK_IMAGE_TYPE_2D;
  image_info.format = format;
  image_info.extent.width = width;
  image_info.extent.height = height;
  image_info.extent.depth = 1;
  image_info.mipLevels = 1;
  image_info.arrayLayers = 1;
  image_info.samples = VK_SAMPLE_COUNT_1_BIT;
  image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
  image_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
  image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  image_info.queueFamilyIndexCount = 0;
  image_info.pQueueFamilyIndices = nullptr;

  VkResult result = device_->vkCreateImage(&image_info, nullptr, &buffer->image);
  if (result != VK_SUCCESS) {
    throw runtime_error("Could not create GBuffer image.");
  }

  device_->vkGetImageMemoryRequirements(buffer->image, &buffer->mem_req);
}

void GBuffer::allocate_memory_(void) {
  uint32_t mem_type_bits = ~0;
  VkDeviceSize mem_size = 0;
  for_each(buffers_.begin(), buffers_.end(), [&mem_type_bits, &mem_size](Buffer& buf)
  {
    mem_type_bits &= buf.mem_req.memoryTypeBits;
    buf.mem_offset = buf.mem_req.alignment * static_cast<VkDeviceSize>(ceil(mem_size / static_cast<double>(buf.mem_req.alignment)));
    mem_size = buf.mem_offset + buf.mem_req.size;
  });

  VkMemoryPropertyFlagBits desired_props = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
  auto& mem_props = device_->physical_device()->memory_properties();
  for (uint32_t i = 0; i < mem_props.memoryTypeCount; ++i) {
    // Current memory type (i) suitable and the memory type has desired properties.
    if ((mem_type_bits & (1 << i)) && ((mem_props.memoryTypes[i].propertyFlags & desired_props) == desired_props)) {
      VkMemoryAllocateInfo alloc_info = {};
      alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
      alloc_info.pNext = nullptr;
      alloc_info.allocationSize = mem_size;
      alloc_info.memoryTypeIndex = i;

      VkResult result = device_->vkAllocateMemory(&alloc_info, nullptr, &memory_);
      if (result == VK_SUCCESS) {
        return;
      }
    }
  }

  throw runtime_error("Could not allocate GBuffer memory.");
}

void GBuffer::bind_buffers_to_memory_(void) {
  for_each(buffers_.begin(), buffers_.end(), [this](const Buffer& buf)
  {
    VkResult result = device_->vkBindImageMemory(buf.image, memory_, buf.mem_offset);
    if (result != VK_SUCCESS) {
      throw runtime_error("Could not bind GBuffer image memory.");
    }
  });
}

void GBuffer::create_image_views_(void) {
  for_each(buffers_.begin(), buffers_.end(), [this](Buffer& buf)
  {
    VkImageViewCreateInfo view_info = {};
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.pNext = nullptr;
    view_info.flags = 0;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = buf.format;
    view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.image = buf.image;
    view_info.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };

    VkResult result = device_->vkCreateImageView(&view_info, nullptr, &buf.view);
    if (result != VK_SUCCESS) {
      throw runtime_error("Could not create GBuffer image view.");
    }
  });
}

} // deferred_shading
} // graphics
