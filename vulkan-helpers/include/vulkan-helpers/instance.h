#pragma once

#include "vk_dispatch_tables.h"

#include <vector>

namespace vk {

class Instance : public vkgen::InstanceFunctions {
public:
  Instance(VkInstance instance, vkgen::GlobalFunctions* globals);
  std::vector<VkPhysicalDevice> physical_devices() const;

private:
  Instance(VkInstance& other) = delete;
  void operator=(VkInstance& rhs) = delete;
};

}
