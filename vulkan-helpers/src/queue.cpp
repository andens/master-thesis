#include "queue.h"

#include "device.h"

namespace vk {

Queue::Queue(VkQueue queue, Device* device)
  : vkgen::QueueFunctions(queue, device) {}

}
