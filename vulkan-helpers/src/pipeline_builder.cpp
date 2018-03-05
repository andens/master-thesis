#include "pipeline_builder.h"

#include "device.h"
#include "pipeline_vertex_layout.h"

namespace vk {

void PipelineBuilder::shader_stage(VkShaderStageFlagBits stage, VkShaderModule shader) {
  VkPipelineShaderStageCreateInfo shader_info = {};
  shader_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shader_info.pNext = nullptr;
  shader_info.flags = 0;
  shader_info.stage = stage;
  shader_info.module = shader;
  // Entry point
  shader_info.pName = "main";
  // Allows to specify values for shader constants.
  shader_info.pSpecializationInfo = nullptr;

  shaders_.push_back(shader_info);
}

void PipelineBuilder::shader_specialization_data(void const* data, size_t data_size) {
  shader_specializations_.push_back({});
  auto& spec_info = shader_specializations_.back().spec_info;
  spec_info.dataSize = data_size;
  spec_info.pData = data;

  auto& shader_stage = shaders_.back();
  shader_stage.pSpecializationInfo = &spec_info;
}

void PipelineBuilder::shader_specialization_map(uint32_t constant_id, uint32_t offset, size_t size) {
  auto& specialization = shader_specializations_.back();

  VkSpecializationMapEntry map_entry {};
  map_entry.constantID = constant_id;
  map_entry.offset = offset;
  map_entry.size = size;
  specialization.map_entries.push_back(map_entry);

  specialization.spec_info.mapEntryCount = static_cast<uint32_t>(specialization.map_entries.size());
  specialization.spec_info.pMapEntries = specialization.map_entries.data();
}

void PipelineBuilder::vertex_layout(const std::function<void(PipelineVertexLayout&)>& definition) {
  vertex_layout_ = std::unique_ptr<PipelineVertexLayout>(new PipelineVertexLayout);
  definition(*vertex_layout_);
}

void PipelineBuilder::ia_triangle_list() {
  input_assembly_info_.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  input_assembly_info_.pNext = nullptr;
  input_assembly_info_.flags = 0;
  input_assembly_info_.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  // When indexed rendering: whether a special index value (max I think)
  // restarts assembly of a given primitive.
  input_assembly_info_.primitiveRestartEnable = VK_FALSE;
}

void PipelineBuilder::vp_dynamic() {
  // Viewport: what part of an image to render to
  // Scissors: restrict fragment generation to a specified area. Scissor test
  // is always enabled in Vulkan, and as such must be specified.
  viewport_info_.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewport_info_.pNext = nullptr;
  viewport_info_.flags = 0;
  // Can specify more viewports, but must enable multiViewport feature
  viewport_info_.viewportCount = 1;
  // Not required when using dynamic state
  viewport_info_.pViewports = nullptr;
  viewport_info_.scissorCount = 1;
  // One for each viewport, not required for dynamic state
  viewport_info_.pScissors = nullptr;
}

void PipelineBuilder::rs_fill_cull_back() {
  // Rasterization configuration (how polygons are rasterized)
  rasterization_info_.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterization_info_.pNext = nullptr;
  rasterization_info_.flags = 0;
  // Depth values beyond near and far plane of frustum: clamp to planes (true)
  // or discard as usual (false). Requires feature.
  rasterization_info_.depthClampEnable = VK_FALSE;
  // If true, geometry is never rasterized (turns off fragment generation).
  rasterization_info_.rasterizerDiscardEnable = VK_FALSE;
  // Feature for other than fill.
  rasterization_info_.polygonMode = VK_POLYGON_MODE_FILL;
  // Maximum depends on hardware. Width greater than 1.0f requires wideLines
  // feature.
  rasterization_info_.lineWidth = 1.0f;
  rasterization_info_.cullMode = VK_CULL_MODE_BACK_BIT;
  // I'm using left-handed where thumb is face direction, ergo clockwise :)
  // However, Vulkan clip space is right handed with Y down, but projection
  // takes care of that by inverting Y.
  rasterization_info_.frontFace = VK_FRONT_FACE_CLOCKWISE;
  rasterization_info_.depthBiasEnable = VK_FALSE;
  rasterization_info_.depthBiasConstantFactor = 0.0f;
  rasterization_info_.depthBiasClamp = 0.0f;
  rasterization_info_.depthBiasSlopeFactor = 0.0f;
}

void PipelineBuilder::ms_none() {
  // Sampling configuration
  multisample_info_.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisample_info_.pNext = nullptr;
  multisample_info_.flags = 0;
  multisample_info_.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  // Shading per sample (enabled) instead of per fragment (disabled).
  // Requires feature.
  multisample_info_.sampleShadingEnable = VK_FALSE;
  // Minimum number of unique sample locations used during fragment shading.
  multisample_info_.minSampleShading = 1.0f;
  // Array of static coverage sample masks.
  multisample_info_.pSampleMask = nullptr;
  // Whether fragment alpha value should be used for coverage.
  multisample_info_.alphaToCoverageEnable = VK_FALSE;
  // Whether fragment alpha should be replaced with one. Requires feature.
  multisample_info_.alphaToOneEnable = VK_FALSE;
}

void PipelineBuilder::ds_enabled() {
  // Depth/stencil configuration
  depth_stencil_info_.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depth_stencil_info_.pNext = nullptr;
  depth_stencil_info_.flags = 0;
  // Compare to depth buffer or not.
  depth_stencil_info_.depthTestEnable = VK_TRUE;
  // Write new depth values if pass.
  depth_stencil_info_.depthWriteEnable = VK_TRUE;
  depth_stencil_info_.depthCompareOp = VK_COMPARE_OP_GREATER_OR_EQUAL;
  // Optional depth bounds test to keep fragments within range.
  depth_stencil_info_.depthBoundsTestEnable = VK_FALSE;
  depth_stencil_info_.minDepthBounds = 0.0f;
  depth_stencil_info_.maxDepthBounds = 1.0f;
  // Not using stencil right now.
  depth_stencil_info_.stencilTestEnable = VK_FALSE;
  depth_stencil_info_.front = {};
  depth_stencil_info_.back = {};
}

void PipelineBuilder::bs_none(uint32_t attachment_count) {
  // Note: If blend builder needs to be expanded in the future it would make
  // sense with a helper class returned similarly as to vertex layout, with
  // methods to define blend info for attachments one by one. This method could
  // remain as a convenience method, but would internally use the helper class
  // by calling a none blend |attachment_count| times and remove the by then
  // deprecated vector of blend attachment states.

  // Blend state for individual attachments.
  blend_attachment_states_.clear();
  blend_attachment_states_.resize(attachment_count);
  for (auto& attachment_state : blend_attachment_states_) {
    // Disabling blending causes all other parameters to be irrelevant except
    // for write mask.
    attachment_state.blendEnable = VK_FALSE;
    attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_COLOR;
    attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
    attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
    attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;
    // Bitmask selecting which of the RGBA components are enabled for writing.
    attachment_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  }

  // Blend state for entire framebuffer.
  // pAttachments: by default all elements must use the same blend state as
  // blending is the same for all attachments. The reason it's an array is that
  // there is an independent blend feature which allows distinct blending for
  // each attachment.
  blend_info_.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  blend_info_.pNext = nullptr;
  blend_info_.flags = 0;
  // Bitwise combination as blending. This disables the other type of blending
  // by effectively setting blendEnable to VK_FALSE.
  blend_info_.logicOpEnable = VK_FALSE;
  // Type of logical operation (copy, clear, and so on).
  blend_info_.logicOp = VK_LOGIC_OP_CLEAR;
  // Must equal number of color attachments defined in render pass.
  blend_info_.attachmentCount = static_cast<uint32_t>(blend_attachment_states_.size());
  // State parameters for each color attachment used in a subpass for which the
  // graphics pipeline is bound. Elements correspond to each color attachment
  // defined in a render pass.
  blend_info_.pAttachments = blend_attachment_states_.data();
  blend_info_.blendConstants[0] = 0.0f;
  blend_info_.blendConstants[1] = 0.0f;
  blend_info_.blendConstants[2] = 0.0f;
  blend_info_.blendConstants[3] = 0.0f;
}

void PipelineBuilder::dynamic_state(std::initializer_list<VkDynamicState> states) {
  dynamic_states_.insert(dynamic_states_.end(), states.begin(), states.end());
}

VkPipeline PipelineBuilder::build(Device& device, VkPipelineLayout pipeline_layout, VkRenderPass render_pass) {
  if (!vertex_layout_) {
    vertex_layout_ = std::unique_ptr<PipelineVertexLayout>(new PipelineVertexLayout);
  }

  VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
  vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertex_input_info.pNext = nullptr;
  vertex_input_info.flags = 0;
  vertex_input_info.vertexBindingDescriptionCount = static_cast<uint32_t>(vertex_layout_->get_bindings().size());
  vertex_input_info.pVertexBindingDescriptions = vertex_layout_->get_bindings().data();
  vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertex_layout_->get_attributes().size());
  vertex_input_info.pVertexAttributeDescriptions = vertex_layout_->get_attributes().data();

  VkPipelineDynamicStateCreateInfo dynamic_state_info = {};
  dynamic_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamic_state_info.pNext = nullptr;
  dynamic_state_info.flags = 0;
  dynamic_state_info.dynamicStateCount = static_cast<uint32_t>(dynamic_states_.size());
  dynamic_state_info.pDynamicStates = dynamic_states_.data();

  VkGraphicsPipelineCreateInfo pipeline_info = {};
  pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipeline_info.pNext = nullptr;
  // Creating a derived pipeline (inheriting another) or if we allow deriving
  // from this one. Can also disable optimizations.
  pipeline_info.flags = 0;
  pipeline_info.stageCount = static_cast<uint32_t>(shaders_.size());
  // Descriptions of active shader stages. At minimum a vertex stage.
  pipeline_info.pStages = shaders_.data();
  pipeline_info.pVertexInputState = &vertex_input_info;
  pipeline_info.pInputAssemblyState = &input_assembly_info_;
  pipeline_info.pTessellationState = nullptr;
  // Can be null if rasterization is disabled.
  pipeline_info.pViewportState = &viewport_info_;
  pipeline_info.pRasterizationState = &rasterization_info_;
  // Can be null if rasterization is disabled.
  pipeline_info.pMultisampleState = &multisample_info_;
  // Can be null if rasterization is disabled or we're not using depth/stencil
  // attachments in a render pass.
  pipeline_info.pDepthStencilState = &depth_stencil_info_;
  // Can be null if rasterization is disabled or when not using any color
  // attachments inside the render pass.
  pipeline_info.pColorBlendState = &blend_info_;
  // Specifies which parts of the graphics pipeline can be set dynamically.
  // Can be null if entire state is static. Note that parameters related to
  // dynamic state is ignored and runtime values may be undefined if we don't
  // use Vulkan commands to change the state.
  pipeline_info.pDynamicState = &dynamic_state_info;
  // Describes resources accessed inside shaders.
  pipeline_info.layout = pipeline_layout;
  // Pipeline can be used with any render pass compatible with this one.
  pipeline_info.renderPass = render_pass;
  // Index of the subpass in which the pipeline will be used.
  pipeline_info.subpass = 0;
  // Pipeline this one should derive from.
  pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
  // Index of a pipeline this one should derive from (when creating multiple
  // pipelines at once, this referes to a previous one in the array).
  pipeline_info.basePipelineIndex = -1;

  VkPipeline pipeline = VK_NULL_HANDLE;
  VkResult result = device.vkCreateGraphicsPipelines(VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &pipeline);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Could not create graphics pipeline.");
  }

  return pipeline;
}

}
