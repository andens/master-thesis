#pragma once

#include <cstdint>
#include <vector>
#include "vulkan_include.inl"

namespace vk {

// Defines how a pipeline is to read vertex data. This includes what vertex
// streams are used, what attributes to use in shaders, and where in a stream
// they get their data.
class PipelineVertexLayout {
public:
  // Define a new stream of vertex buffer data.
  void stream(uint32_t binding, uint32_t stride, VkVertexInputRate input_rate);
  // Define a vertex attribute resident in the most recently added stream.
  void attribute(uint32_t location, VkFormat format, uint32_t offset);

  const std::vector<VkVertexInputBindingDescription>& get_bindings() const;
  const std::vector<VkVertexInputAttributeDescription>& get_attributes() const;

private:
  std::vector<VkVertexInputBindingDescription> vertex_input_bindings_;
  std::vector<VkVertexInputAttributeDescription> vertex_input_attributes_;
};

}
