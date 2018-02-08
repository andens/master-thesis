#include "instance.h"

using namespace std;

namespace vk {

Instance::Instance(VkInstance instance, vkgen::GlobalFunctions* globals) : vkgen::InstanceFunctions(instance, globals) {
  // Here would be a suitable place to cache some stuff related to instance,
  // mayhaps physical devices available?
}

vector<VkPhysicalDevice> Instance::physical_devices() const {
  uint32_t physical_device_count = 0;
  vkEnumeratePhysicalDevices(&physical_device_count, nullptr);
  vector<VkPhysicalDevice> physical_devices(physical_device_count);
  vkEnumeratePhysicalDevices(&physical_device_count, physical_devices.data());
  return physical_devices;
}

}
