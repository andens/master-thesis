#include "descriptor_set.h"

#include "device.h"

namespace vk {

DescriptorSet::DescriptorSet(VkDescriptorSet set)
  : set_(set) {

}

void DescriptorSet::write_combined_image_sampler(uint32_t binding, VkImageView view, VkSampler sampler) {
  VkDescriptorImageInfo image_info {};
  image_info.sampler = sampler;
  image_info.imageView = view;
  image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  image_descriptors_.push_back(image_info);

  VkWriteDescriptorSet desc_write {};
  desc_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  desc_write.pNext = nullptr;
  desc_write.dstSet = set_;
  desc_write.dstBinding = binding;
  desc_write.dstArrayElement = 0;
  desc_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  desc_write.descriptorCount = 1;
  desc_write.pBufferInfo = nullptr;
  desc_write.pImageInfo = nullptr;
  desc_write.pTexelBufferView = nullptr;
  descriptor_writes_.push_back(desc_write);
}

void DescriptorSet::write_storage_buffer(uint32_t binding, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range) {
  // VkDescriptorBufferInfo specifies which buffer and what region within it
  // contains the descriptor data.
  VkDescriptorBufferInfo buffer_info = {};
  buffer_info.buffer = buffer;
  buffer_info.offset = offset;
  buffer_info.range = range;
  buffer_descriptors_.push_back(buffer_info);

  // This struct tells the driver to update a certain number of bindings of
  // some specific type with new data.
  VkWriteDescriptorSet desc_write = {};
  desc_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  desc_write.pNext = nullptr;
  // Which descriptor set to update.
  desc_write.dstSet = set_;
  // The binding within the set to update.
  desc_write.dstBinding = binding;
  // First index to update if the binding is an array.
  desc_write.dstArrayElement = 0;
  desc_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  // How many descriptors we are writing.
  desc_write.descriptorCount = 1;
  // The update configurations (can be more if array). These are set when it's
  // time to write the descriptors.
  desc_write.pBufferInfo = nullptr;
  desc_write.pImageInfo = nullptr;
  desc_write.pTexelBufferView = nullptr;
  descriptor_writes_.push_back(desc_write);
}

void DescriptorSet::write_storage_texel_buffer(uint32_t binding, VkBufferView view) {
  texel_buffer_views_.push_back(view);

  VkWriteDescriptorSet desc_write = {};
  desc_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  desc_write.pNext = nullptr;
  desc_write.dstSet = set_;
  desc_write.dstBinding = binding;
  desc_write.dstArrayElement = 0;
  desc_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
  desc_write.descriptorCount = 1;
  desc_write.pBufferInfo = nullptr;
  desc_write.pImageInfo = nullptr;
  desc_write.pTexelBufferView = nullptr;
  descriptor_writes_.push_back(desc_write);
}

void DescriptorSet::write_uniform_buffer(uint32_t binding, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range) {
  VkDescriptorBufferInfo buffer_info = {};
  buffer_info.buffer = buffer;
  buffer_info.offset = offset;
  buffer_info.range = range;
  buffer_descriptors_.push_back(buffer_info);

  VkWriteDescriptorSet desc_write = {};
  desc_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  desc_write.pNext = nullptr;
  desc_write.dstSet = set_;
  desc_write.dstBinding = binding;
  desc_write.dstArrayElement = 0;
  desc_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  desc_write.descriptorCount = 1;
  desc_write.pBufferInfo = nullptr;
  desc_write.pImageInfo = nullptr;
  desc_write.pTexelBufferView = nullptr;
  descriptor_writes_.push_back(desc_write);
}

void DescriptorSet::update_pending(vk::Device& device) {
  // Set pointers as appropriate
  uint32_t currentBufferInfo = 0;
  uint32_t currentImageInfo = 0;
  uint32_t currentTexelBufferView = 0;
  for (auto& write : descriptor_writes_) {
    switch (write.descriptorType) {
      case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER: {
        write.pBufferInfo = &buffer_descriptors_[currentBufferInfo];
        currentBufferInfo += write.descriptorCount;
        break;
      }
      case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER: {
        write.pTexelBufferView = &texel_buffer_views_[currentTexelBufferView];
        currentTexelBufferView += write.descriptorCount;
        break;
      }
      case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER: {
        write.pBufferInfo = &buffer_descriptors_[currentBufferInfo];
        currentBufferInfo += write.descriptorCount;
        break;
      }
      case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: {
        write.pImageInfo = &image_descriptors_[currentImageInfo];
        currentImageInfo += write.descriptorCount;
        break;
      }
      default: {
        throw std::runtime_error("DescriptorSet::update_pending : Unhandled descriptor type " + std::to_string(write.descriptorType) + ".");
      }
    }
  }

  device.vkUpdateDescriptorSets(static_cast<uint32_t>(descriptor_writes_.size()), descriptor_writes_.data(), 0, nullptr);
  descriptor_writes_.clear();
  buffer_descriptors_.clear();
  image_descriptors_.clear();
  texel_buffer_views_.clear();
}

}
