#include "pipeline_vertex_layout.h"

namespace vk {

void PipelineVertexLayout::stream(uint32_t binding, uint32_t stride, VkVertexInputRate input_rate) {
  VkVertexInputBindingDescription binding_desc = {};
  binding_desc.binding = binding; // The input stream number
  binding_desc.stride = stride; // Size of an input element (total if multiple attributes)
  binding_desc.inputRate = input_rate; // Vertex or instance frequency
  vertex_input_bindings_.push_back(binding_desc);
}

void PipelineVertexLayout::attribute(uint32_t location, VkFormat format, uint32_t offset) {
  VkVertexInputAttributeDescription attr_desc = {};
  attr_desc.location = location; // Identifier that a shader uses to get this particular attribute
  attr_desc.binding = vertex_input_bindings_.back().binding; // What binding to read data from
  attr_desc.format = format; // Attribute components and their size
  attr_desc.offset = offset; // Offset within one stride of binding
  vertex_input_attributes_.push_back(attr_desc);
}

const std::vector<VkVertexInputBindingDescription>& PipelineVertexLayout::get_bindings() const {
  return vertex_input_bindings_;
}

const std::vector<VkVertexInputAttributeDescription>& PipelineVertexLayout::get_attributes() const {
  return vertex_input_attributes_;
}

}
