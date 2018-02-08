#include "pipeline_layout_builder.h"

#include "device.h"

namespace vk {

void PipelineLayoutBuilder::descriptor_layout(VkDescriptorSetLayout layout) {
  set_layouts_.push_back(layout);
}

void PipelineLayoutBuilder::push_constant(VkShaderStageFlags shader_stages, uint32_t offset, uint32_t size) {
  // A special, small amount of memory is reserved for push constants, which is
  // another way to use shader constants. Push constants are stored in (and
  // updated through) the command buffer and provide enough bytes to hold some
  // matrices or index values. Interpreting the raw data is up to the shader.
  // Updating these values are expected to be faster than regular memory writes.
  VkPushConstantRange push_constant = {};
  push_constant.offset = offset;
  push_constant.size = size;
  push_constant.stageFlags = shader_stages;
  push_constants_.push_back(push_constant);
}

VkPipelineLayout PipelineLayoutBuilder::build(Device& device) {
  VkPipelineLayoutCreateInfo layout_info = {};
  layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  layout_info.pNext = nullptr;
  layout_info.flags = 0;
  layout_info.setLayoutCount = set_layouts_.size();
  layout_info.pSetLayouts = set_layouts_.data(); // Descriptions of the descriptor sets used by pipeline
  layout_info.pushConstantRangeCount = push_constants_.size();
  layout_info.pPushConstantRanges = push_constants_.data();

  VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
  VkResult result = device.vkCreatePipelineLayout(&layout_info, nullptr, &pipeline_layout);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Could not create pipeline layout.");
  }

  return pipeline_layout;
}

}
