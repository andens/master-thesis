#pragma once

#include <cstdint>
#include <functional>
#include <vector>
#include "vulkan_include.inl"

namespace vk {

class Device;

class DescriptorSetLayoutBuilder {
public:
  void sampler(VkShaderStageFlags shader_stages, uint32_t count);
  void combined_image_sampler(VkShaderStageFlags shader_stages, uint32_t count, VkSampler const* sampler);
  void storage_buffer(VkShaderStageFlags shader_stages, uint32_t count);
  void storage_texel_buffer(VkShaderStageFlags shader_stages, uint32_t count);
  void uniform_buffer(VkShaderStageFlags shader_stages, uint32_t count);
  VkDescriptorSetLayout build(Device& device);

private:
  // Build the next binding of the descriptor set layout. Binding data is saved
  // for later final construction of the layout. Binding number is tracked, but
  // additional binding details may be specified via the provided closure.
  void next_binding(const std::function<void(VkDescriptorSetLayoutBinding& binding)>& details);

private:
  uint32_t current_binding_ = 0;
  std::vector<VkDescriptorSetLayoutBinding> bindings_;
};

}
