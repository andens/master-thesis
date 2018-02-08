#pragma once

#include <cstdint>
#include <vector>
#include "vulkan_include.inl"

namespace vk {

class Device;

// The pipeline layout tells what kinds of descriptor sets will be used in a
// pipeline (as a pipeline may have multiple sets bound at once). Multiple sets
// may be used for updating descriptors at different frequencies.
class PipelineLayoutBuilder {
public:
  void descriptor_layout(VkDescriptorSetLayout layout);
  void push_constant(VkShaderStageFlags shader_stages, uint32_t offset, uint32_t size);
  VkPipelineLayout build(Device& device);

private:
  std::vector<VkDescriptorSetLayout> set_layouts_;
  std::vector<VkPushConstantRange> push_constants_;
};

}
