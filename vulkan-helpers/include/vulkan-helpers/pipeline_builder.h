#pragma once

#include <cstdint>
#include <functional>
#include <initializer_list>
#include <memory>
#include <vector>
#include "vulkan_include.inl"

namespace vk {

class Device;
class PipelineVertexLayout;

// In Vulkan we have graphics and compute pipelines. Unlike previous APIs like
// D3D11, all pipeline state is gathered in one object and is immutable,
// meaning that we can't change some particular state separately. This is done
// because now all state and error checking is done when preparing the state,
// not when changing it during runtime. In the case of Vulkan, the pipeline
// describes how to draw (rendering state, depth test, shaders, etc.).
class PipelineBuilder {
public:
  void shader_stage(VkShaderStageFlagBits stage, VkShaderModule shader);
  // Define vertex streams, attributes, and how the attributes get data from streams.
  void vertex_layout(const std::function<void(PipelineVertexLayout&)>& definition);
  // ia: input assembly, the topology of how vertices are assembled into primitives
  void ia_triangle_list();
  // vp: viewport
  void vp_dynamic();
  // rs: rasterizer state
  void rs_fill_cull_back();
  // ms: multisample state
  void ms_none();
  // ds: depth/stencil
  void ds_enabled();
  // bs: blend state
  void bs_none(uint32_t attachment_count);
  void dynamic_state(std::initializer_list<VkDynamicState> states);
  VkPipeline build(Device& device, VkPipelineLayout pipeline_layout, VkRenderPass render_pass);

private:
  std::vector<VkPipelineShaderStageCreateInfo> shaders_;
  std::unique_ptr<PipelineVertexLayout> vertex_layout_ = nullptr;
  VkPipelineInputAssemblyStateCreateInfo input_assembly_info_ = {};
  VkPipelineViewportStateCreateInfo viewport_info_ = {};
  VkPipelineRasterizationStateCreateInfo rasterization_info_ = {};
  VkPipelineMultisampleStateCreateInfo multisample_info_ = {};
  VkPipelineDepthStencilStateCreateInfo depth_stencil_info_ = {};
  std::vector<VkPipelineColorBlendAttachmentState> blend_attachment_states_;
  VkPipelineColorBlendStateCreateInfo blend_info_ = {};
  std::vector<VkDynamicState> dynamic_states_;
};

}
