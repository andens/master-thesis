#include "depth_buffer.h"

#include <vulkan-helpers/command_buffer.h>
#include <vulkan-helpers/device.h>
#include <vulkan-helpers/physical_device.h>
#include <vulkan-helpers/queue.h>

namespace graphics {

DepthBuffer::DepthBuffer(vk::Device* device, uint32_t width, uint32_t height, VkFormat depth_format, vk::CommandBuffer& gfx_cmd_buf, vk::Queue& gfx_queue)
  : device_(device), width_(width), height_(height), format_(depth_format) {
  create_image();
  create_view();
  change_image_layout(gfx_cmd_buf, gfx_queue);
}

DepthBuffer::~DepthBuffer() {
  if (buffer_image_view_) {
    device_->vkDestroyImageView(buffer_image_view_, nullptr);
    buffer_image_view_ = VK_NULL_HANDLE;
  }

  if (buffer_image_memory_) {
    device_->vkFreeMemory(buffer_image_memory_, nullptr);
    buffer_image_memory_ = VK_NULL_HANDLE;
  }

  if (buffer_image_) {
    device_->vkDestroyImage(buffer_image_, nullptr);
    buffer_image_ = VK_NULL_HANDLE;
  }
}

void DepthBuffer::create_image(void) {
  VkImageCreateInfo image_info = {};
  image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  image_info.pNext = nullptr;
  image_info.imageType = VK_IMAGE_TYPE_2D;
  image_info.extent.width = width_;
  image_info.extent.height = height_;
  image_info.extent.depth = 1;
  image_info.mipLevels = 1;
  image_info.arrayLayers = 1;
  image_info.format = format_;
  // Linear: texels are row-major (which is how stb stores as well).
  // Optimal: texels are laid out in an implementation defined order.
  // Tiling can't be changed; we use linear for mapping and optimal when Vulkan
  // copies to device-local memory.
  image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
  // Undefined: first transition discard texels.
  // Preinitialized: first transition preserves texels
  image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  image_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
  image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  image_info.samples = VK_SAMPLE_COUNT_1_BIT; // Only relevant for attachments
  image_info.flags = 0; // Sparse textures and whatnot
  image_info.queueFamilyIndexCount = 0;
  image_info.pQueueFamilyIndices = nullptr; // Ignored when not using concurrent mode

  VkResult result = device_->vkCreateImage(&image_info, nullptr, &buffer_image_);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Could not create depth buffer image.");
  }

  allocate_image_memory();

  result = device_->vkBindImageMemory(buffer_image_, buffer_image_memory_, 0);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Could not bind depth buffer image to memory.");
  }
}

void DepthBuffer::allocate_image_memory(void) {
  VkMemoryRequirements mem_req;
  device_->vkGetImageMemoryRequirements(buffer_image_, &mem_req);

  VkPhysicalDeviceMemoryProperties memory_properties = device_->physical_device()->memory_properties();

  bool allocated = false;
  for (uint32_t i = 0; i < memory_properties.memoryTypeCount; ++i) {
    // Current memory type supports the buffer and we can map it.
    VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    if ((mem_req.memoryTypeBits & (1 << i)) && ((memory_properties.memoryTypes[i].propertyFlags & properties) == properties)) {
      VkMemoryAllocateInfo memory_allocate_info = {};
      memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
      memory_allocate_info.pNext = nullptr;
      memory_allocate_info.allocationSize = mem_req.size;
      memory_allocate_info.memoryTypeIndex = i;

      VkResult result = device_->vkAllocateMemory(&memory_allocate_info, nullptr, &buffer_image_memory_);
      if (result == VK_SUCCESS) {
        allocated = true;
        break;
      }
    }
  }

  if (!allocated) {
    throw std::runtime_error("Could not allocate memory for depth buffer image.");
  }
}

void DepthBuffer::create_view(void) {
  VkImageViewCreateInfo view_info = {};
  view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  view_info.image = buffer_image_;
  view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
  view_info.format = format_;
  view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
  view_info.subresourceRange.baseMipLevel = 0;
  view_info.subresourceRange.levelCount = 1;
  view_info.subresourceRange.baseArrayLayer = 0;
  view_info.subresourceRange.layerCount = 1;
  // view_info.components; Identity swizzle default

  VkResult result = device_->vkCreateImageView(&view_info, nullptr, &buffer_image_view_);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Could not create depth image view.");
  }
}

void DepthBuffer::change_image_layout(vk::CommandBuffer& gfx_cmd_buf, vk::Queue& gfx_queue) {
  VkCommandBufferBeginInfo begin_info = {};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  gfx_cmd_buf.vkBeginCommandBuffer(&begin_info);

  VkImageMemoryBarrier layout_barrier = {};
  layout_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  layout_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  layout_barrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  layout_barrier.image = buffer_image_;
  layout_barrier.srcAccessMask = 0;
  layout_barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
  layout_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  layout_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  layout_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
  layout_barrier.subresourceRange.baseMipLevel = 0;
  layout_barrier.subresourceRange.levelCount = 1;
  layout_barrier.subresourceRange.baseArrayLayer = 0;
  layout_barrier.subresourceRange.layerCount = 1;

  gfx_cmd_buf.vkCmdPipelineBarrier(
    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
    VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
    0,
    0, nullptr,
    0, nullptr,
    1, &layout_barrier);

  gfx_cmd_buf.vkEndCommandBuffer();

  auto cmd_buf = gfx_cmd_buf.command_buffer();
  VkSubmitInfo submit_info = {};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &cmd_buf;
  submit_info.waitSemaphoreCount = 0;
  submit_info.pWaitSemaphores = nullptr;
  submit_info.pWaitDstStageMask = nullptr;
  submit_info.signalSemaphoreCount = 0;
  submit_info.pSignalSemaphores = nullptr;

  gfx_queue.vkQueueSubmit(1, &submit_info, VK_NULL_HANDLE);

  gfx_queue.vkQueueWaitIdle();
}

}
