#pragma once

#include "device_resource.h"
#include "vulkan_include.inl"

namespace vk {

class DescriptorSetLayoutBuilder;
class Device;

class DescriptorSetLayout : public DeviceResource {
public:
  DescriptorSetLayout(Device& device, DescriptorSetLayoutBuilder& builder);
  void destroy(Device& device) override;
  VkDescriptorSetLayout vulkan_handle() const { return layout_; }

private:
  VkDescriptorSetLayout layout_ = VK_NULL_HANDLE;
};

}
