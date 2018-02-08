#include "descriptor_set_layout.h"

#include "descriptor_set_layout_builder.h"
#include "device.h"

namespace vk {

DescriptorSetLayout::DescriptorSetLayout(Device& device, DescriptorSetLayoutBuilder& builder) {
  layout_ = builder.build(device);
}

void DescriptorSetLayout::destroy(Device& device) {
  if (layout_) {
    device.vkDestroyDescriptorSetLayout(layout_, nullptr);
    layout_ = VK_NULL_HANDLE;
  }
}

}
