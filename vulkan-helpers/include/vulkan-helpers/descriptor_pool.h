#pragma once

#include "device_resource.h"

#include <memory>
#include "vulkan_include.inl"

namespace vk {

class DescriptorPoolBuilder;
class DescriptorSet;
class DescriptorSetLayout;
class Device;

class DescriptorPool : public DeviceResource {
public:
  DescriptorPool(Device& device, DescriptorPoolBuilder& builder);
  void destroy(Device& device) override;
  std::unique_ptr<DescriptorSet> allocate(Device& device, DescriptorSetLayout& layout);

private:
  VkDescriptorPool pool_ = VK_NULL_HANDLE;
};

}
