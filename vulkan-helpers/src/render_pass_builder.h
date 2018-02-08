#pragma once

#include <functional>
#include <vector>
#include "vulkan_include.inl"

namespace vk {

class Device;

// c: color | d: depth only
struct RenderPassAttachment : public VkAttachmentDescription {
  RenderPassAttachment();
  static RenderPassAttachment c_clear_store(VkFormat format);
  static RenderPassAttachment d_clear_store(VkFormat format);
};

class Subpass {
  friend class RenderPassBuilder;

public:
  Subpass(VkPipelineBindPoint bind_point);
  // Used to make an attachment shader input (outside descriptor set, this can
  // only work with the corresponding element that is being processed).
  void input_attachment(uint32_t attachment, VkImageLayout layout);
  // Render target (fragment shader references this array when using
  // `layout(location = 0) out vec4 outColor`). Each color attachment may have
  // a corresponding resolve attachment, into which the color attachment will
  // be resolved. It's used for multisampling or something.
  void color_attachment(uint32_t attachment, VkImageLayout layout, uint32_t resolve_attachment = VK_ATTACHMENT_UNUSED, VkImageLayout resolve_layout = VK_IMAGE_LAYOUT_UNDEFINED);
  // The attachment used as depth/stencil
  void depth_stencil_attachment(uint32_t attachment, VkImageLayout layout);
  // If we don't use an attachment but need contents in a later subpass, we
  // indicate so here so it's not tampered with.
  void preserve_attachment(uint32_t attachment);

private:
  VkSubpassDescription description_;
  std::vector<VkAttachmentReference> input_attachments_ = {};
  std::vector<VkAttachmentReference> color_attachments_ = {};
  std::vector<VkAttachmentReference> resolve_attachments_ = {};
  VkAttachmentReference depth_stencil_ = {};
  std::vector<uint32_t> preserve_attachments_ = {};
};

// A render pass is a set of attachments (images) required for rendering
// operations and a collection of subpasses using those attachments in some
// way. We declare in what layout the attachments are provided together with
// what layouts the attachments should be in when used by the various
// subpasses. Hardware performs transitions as necessary. Note that the render
// pass only describes formats of attachments and how they will be used, but
// not the actual resources to use. That belong in the framebuffer. The render
// pass is a construct that collects attachments and operations modifying them
// so that the driver doesn't have to deduce it.
class RenderPassBuilder {
public:
  void attachment(RenderPassAttachment const& attachment_description);
  void graphics_subpass(std::function<void(Subpass&)> const& manipulator);
  // Dependencies between pairs of subpasses. Layouts themselves do not tell
  // the whole story about usage, and this information helps the driver
  // optimize for example barriers (like if we write into images only in a
  // vertex shader, we don't need to wait until the fragment shader executes).
  void subpass_dependency(VkSubpassDependency const& dependency);
  VkRenderPass build(Device& device);

private:
  std::vector<VkAttachmentDescription> attachments_;
  std::vector<Subpass> subpasses_;
  std::vector<VkSubpassDependency> subpass_dependencies_;
};

}
