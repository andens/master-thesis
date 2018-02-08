#include "render_pass_builder.h"

#include "device.h"

namespace vk {

RenderPassAttachment::RenderPassAttachment() {
  flags = 0; // Only aliasing for now
  samples = VK_SAMPLE_COUNT_1_BIT;
  // What to do with image contents at beginning of render pass.
  loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  // What to do after the last subpass that used the image. Here we preserve contents.
  storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  // If the attachment is depth/stencil, this is just the stencil part (depth is treated in loadOp)
  stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  // What layout we expect attachments to be at the beginning of the render pass.
  initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  // Layout to transition to at the end of the render pass. Hardware does
  // this automatically if required.
  finalLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  format = VK_FORMAT_UNDEFINED;
}

RenderPassAttachment RenderPassAttachment::c_clear_store(VkFormat format) {
  RenderPassAttachment att;
  att.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  att.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  att.finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
  att.format = format;
  return att;
}

RenderPassAttachment RenderPassAttachment::d_clear_store(VkFormat format) {
  RenderPassAttachment att;
  att.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  att.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  att.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  att.format = format;
  return att;
}

Subpass::Subpass(VkPipelineBindPoint bind_point) {
  description_.flags = 0; // Future use
  // What kind of pipeline this subpass will be used in (graphics/compute)
  description_.pipelineBindPoint = bind_point;
  description_.inputAttachmentCount = 0;
  description_.pInputAttachments = nullptr;
  description_.colorAttachmentCount = 0;
  description_.pColorAttachments = nullptr;
  description_.pResolveAttachments = nullptr;
  description_.pDepthStencilAttachment = nullptr;
  description_.preserveAttachmentCount = 0;
  description_.pPreserveAttachments = nullptr;
}

void Subpass::input_attachment(uint32_t attachment, VkImageLayout layout) {
  input_attachments_.push_back({ attachment, layout });
  description_.inputAttachmentCount = static_cast<uint32_t>(input_attachments_.size());
  description_.pInputAttachments = input_attachments_.data();
}

void Subpass::color_attachment(uint32_t attachment, VkImageLayout layout, uint32_t resolve_attachment, VkImageLayout resolve_layout) {
  color_attachments_.push_back({ attachment, layout });
  description_.colorAttachmentCount = static_cast<uint32_t>(color_attachments_.size());
  description_.pColorAttachments = color_attachments_.data();

  resolve_attachments_.push_back({ resolve_attachment, resolve_layout });
  description_.pResolveAttachments = resolve_attachments_.data();
}

void Subpass::depth_stencil_attachment(uint32_t attachment, VkImageLayout layout) {
  depth_stencil_ = { attachment, layout };
  description_.pDepthStencilAttachment = &depth_stencil_;
}

void Subpass::preserve_attachment(uint32_t attachment) {
  preserve_attachments_.push_back(attachment);
  description_.preserveAttachmentCount = static_cast<uint32_t>(preserve_attachments_.size());
  description_.pPreserveAttachments = preserve_attachments_.data();
}

// We begin by describing the attachments (the various buffers) used in one way
// or another during the render pass. This is a general array that subpasses
// will reference into.
void RenderPassBuilder::attachment(RenderPassAttachment const& attachment_description) {
  attachments_.push_back(attachment_description);
}

void RenderPassBuilder::graphics_subpass(std::function<void(Subpass&)> const& manipulator) {
  subpasses_.push_back(Subpass(VK_PIPELINE_BIND_POINT_GRAPHICS));
  manipulator(subpasses_.back());
}

void RenderPassBuilder::subpass_dependency(VkSubpassDependency const& dependency) {
  subpass_dependencies_.push_back(dependency);
}

VkRenderPass RenderPassBuilder::build(Device& device) {
  std::vector<VkSubpassDescription> subpass_descriptions;
  subpass_descriptions.reserve(subpasses_.size());
  for (auto& s : subpasses_) {
    subpass_descriptions.push_back(s.description_);
  }

  VkRenderPassCreateInfo render_pass_info = {};
  render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  render_pass_info.flags = 0; // Future use
  render_pass_info.attachmentCount = static_cast<uint32_t>(attachments_.size());
  render_pass_info.pAttachments = attachments_.data();
  render_pass_info.subpassCount = static_cast<uint32_t>(subpass_descriptions.size());
  render_pass_info.pSubpasses = subpass_descriptions.data();
  render_pass_info.dependencyCount = static_cast<uint32_t>(subpass_dependencies_.size());
  render_pass_info.pDependencies = subpass_dependencies_.data();

  VkRenderPass render_pass = VK_NULL_HANDLE;
  VkResult result = device.vkCreateRenderPass(&render_pass_info, nullptr, &render_pass);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Could not create render pass.");
  }

  return render_pass;
}

}
