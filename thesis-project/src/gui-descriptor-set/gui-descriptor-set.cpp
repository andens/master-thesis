#include "gui-descriptor-set.h"

#include <vulkan-helpers/descriptor_pool.h>
#include <vulkan-helpers/descriptor_set.h>
#include <vulkan-helpers/descriptor_set_layout.h>
#include <vulkan-helpers/descriptor_set_layout_builder.h>
#include <vulkan-helpers/device.h>

void GuiDescriptorSet::destroy(vk::Device& device) {
  layout_->destroy(device);
}

vk::DescriptorSetLayout const& GuiDescriptorSet::layout() const {
  return *layout_;
}

vk::DescriptorSet const& GuiDescriptorSet::set() const {
  return *set_;
}

void GuiDescriptorSet::use_font_image(vk::Device& device, VkImageView view, VkSampler sampler) {
  set_->write_combined_image_sampler(0, view, sampler);
  set_->update_pending(device);
}

GuiDescriptorSet::GuiDescriptorSet(vk::Device& device, vk::DescriptorPool& pool, VkSampler sampler) {
  vk::DescriptorSetLayoutBuilder layout_builder;
  layout_builder.combined_image_sampler(VK_SHADER_STAGE_FRAGMENT_BIT, 1, &sampler);
  layout_.reset(new vk::DescriptorSetLayout{ device, layout_builder });
  set_ = pool.allocate(device, *layout_);
}

GuiDescriptorSet::~GuiDescriptorSet() {}
