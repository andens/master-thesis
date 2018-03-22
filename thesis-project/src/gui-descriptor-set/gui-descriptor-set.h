#pragma once

#include <memory>
#include <vulkan-helpers/vulkan_include.inl>

namespace vk {
  class DescriptorPool;
  class DescriptorSet;
  class DescriptorSetLayout;
  class Device;
}

class GuiDescriptorSet {
public:
  void destroy(vk::Device& device);
  vk::DescriptorSetLayout const& layout() const;
  vk::DescriptorSet const& set() const;
  void use_font_image(vk::Device& device, VkImageView view, VkSampler sampler);

  GuiDescriptorSet(vk::Device& device, vk::DescriptorPool& pool, VkSampler sampler);
  ~GuiDescriptorSet();

private:
  GuiDescriptorSet(GuiDescriptorSet& other) = delete;
  void operator=(GuiDescriptorSet& rhs) = delete;

private:
  std::unique_ptr<vk::DescriptorSetLayout> layout_;
  std::unique_ptr<vk::DescriptorSet> set_;
};
