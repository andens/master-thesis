#include "descriptor_set_layout_builder.h"

#include "device.h"

namespace vk {

void DescriptorSetLayoutBuilder::sampler(VkShaderStageFlags shader_stages, uint32_t count) {
  next_binding([shader_stages, count](auto& binding) {
    binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    binding.descriptorCount = count;
    binding.stageFlags = shader_stages;
  });
}

void DescriptorSetLayoutBuilder::storage_buffer(VkShaderStageFlags shader_stages, uint32_t count) {
  next_binding([shader_stages, count](auto& binding) {
    binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    binding.descriptorCount = count;
    binding.stageFlags = shader_stages;
  });
}

void DescriptorSetLayoutBuilder::storage_texel_buffer(VkShaderStageFlags shader_stages, uint32_t count) {
  next_binding([shader_stages, count](auto& binding) {
    binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
    binding.descriptorCount = count;
    binding.stageFlags = shader_stages;
  });
}

void DescriptorSetLayoutBuilder::uniform_buffer(VkShaderStageFlags shader_stages, uint32_t count) {
  next_binding([shader_stages, count](auto& binding) {
    binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    binding.descriptorCount = count;
    binding.stageFlags = shader_stages;
  });
}

VkDescriptorSetLayout DescriptorSetLayoutBuilder::build(Device& device) {
  VkDescriptorSetLayoutCreateInfo layout_info = {};
  layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layout_info.pNext = nullptr;
  layout_info.flags = 0;
  layout_info.bindingCount = bindings_.size();
  layout_info.pBindings = bindings_.data();

  VkDescriptorSetLayout set_layout = VK_NULL_HANDLE;
  VkResult result = device.vkCreateDescriptorSetLayout(&layout_info, nullptr, &set_layout);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Could not create descriptor set layout.");
  }

  return set_layout;
}

void DescriptorSetLayoutBuilder::next_binding(const std::function<void(VkDescriptorSetLayoutBinding& binding)>& details) {
  VkDescriptorSetLayoutBinding binding = {};
  // What kind of descriptor this binding represents.
  binding.descriptorType = VK_DESCRIPTOR_TYPE_BEGIN_RANGE;
  // For array. Could be used for skeletal animation.
  binding.descriptorCount = 1;
  // Shader stages in which the descriptor will be referenced.
  binding.stageFlags = 0;
  // Texture related descriptors.
  binding.pImmutableSamplers = nullptr;

  details(binding);
  binding.binding = current_binding_; // Matches shader

  bindings_.push_back(binding);
  current_binding_++;
}

}
