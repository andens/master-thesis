#include "command_buffer.h"

#include "device.h"

using namespace std;

namespace vk {

CommandBuffer::CommandBuffer(VkCommandBuffer cmd_buf, weak_ptr<Device> device)
  : vkgen::CommandBufferFunctions(cmd_buf, device.lock().get()), device_(device) {}

}
