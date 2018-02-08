#pragma once

#include <memory>
#include "vk_dispatch_tables.h"

namespace vk {

class PhysicalDevice;
class Queue;

class Device : public vkgen::DeviceFunctions {
  friend class DeviceBuilder;

public:
  Device(VkDevice device, std::shared_ptr<PhysicalDevice> phys_dev, vkgen::InstanceFunctions* instance) : vkgen::DeviceFunctions(device, instance), phys_dev_(phys_dev) {}

  std::shared_ptr<PhysicalDevice> physical_device() { return phys_dev_; }
  std::shared_ptr<Queue> graphics_queue() { return graphics_queue_; }
  std::shared_ptr<Queue> compute_queue() { return compute_queue_; }
  std::shared_ptr<Queue> present_queue() { return present_queue_; }
  uint32_t graphics_family() { return graphics_family_; }
  uint32_t compute_family() { return compute_family_; }
  uint32_t present_family() { return present_family_; }

  VkSemaphore create_semaphore();
  VkFence create_fence(bool signaled = false);

private:
  std::shared_ptr<PhysicalDevice> phys_dev_ = nullptr;
  std::shared_ptr<Queue> graphics_queue_ = nullptr;
  std::shared_ptr<Queue> compute_queue_ = nullptr;
  std::shared_ptr<Queue> present_queue_ = nullptr;
  uint32_t graphics_family_ = UINT32_MAX;
  uint32_t compute_family_ = UINT32_MAX;
  uint32_t present_family_ = UINT32_MAX;
};

}
