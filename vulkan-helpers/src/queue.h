#pragma once

#include "vk_dispatch_tables.h"

namespace vk {

class Device;

class Queue : public vkgen::QueueFunctions {
public:
  Queue(VkQueue queue, Device* device);
};

}
