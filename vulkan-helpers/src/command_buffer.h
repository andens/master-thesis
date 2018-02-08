#pragma once

#include <memory>
#include "vk_dispatch_tables.h"

namespace vk {

class Device;

class CommandBuffer : public vkgen::CommandBufferFunctions {
public:
  CommandBuffer(VkCommandBuffer cmd_buf, std::weak_ptr<Device> device);
  std::weak_ptr<Device> device() { return device_; }

private:
  std::weak_ptr<Device> device_;
};

}
