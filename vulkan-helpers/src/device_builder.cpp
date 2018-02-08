#include "device_builder.h"

#include "instance.h"
#include "physical_device.h"
#include "device.h"
#include "queue.h"

using namespace std;

namespace vk {

//void DeviceBuilder::with_graphics_queues(uint32_t count) {
//  graphics_queues_ = count;
//}
//
//void DeviceBuilder::with_compute_queues(uint32_t count) {
//  compute_queues_ = count;
//}
//
//void DeviceBuilder::with_present_queues(uint32_t count) {
//  present_queues_ = count;
//}

void DeviceBuilder::use_extension(char const* extension) {
  extensions_.push_back(extension);
}

unique_ptr<Device> DeviceBuilder::build(Instance* instance) {
  // We must find a physical device (an actual GPU) that supports rendering
  // and presentation.
  auto physical_devices = instance->physical_devices();

  // Earlier I used to check if VK_KHR_swapchain extension was available on a
  // physical device, but since it's the device companion to VK_KHR_surface
  // presentation support should imply availability (and frankly it's common
  // enough that it shouldn't be a problem).

  shared_ptr<PhysicalDevice> physical_device = nullptr;
  uint32_t graphics_family = UINT32_MAX;
  uint32_t compute_family = UINT32_MAX;
  uint32_t present_family = UINT32_MAX;
  for (uint32_t i = 0; i < physical_devices.size(); ++i) {
    auto phys_dev = unique_ptr<PhysicalDevice>(new PhysicalDevice(physical_devices[i], instance));
    if (is_physical_device_suitable(phys_dev.get(), graphics_family, compute_family, present_family)) {
      physical_device = move(phys_dev);
      break;
    }
  }

  if (!physical_device) {
    throw runtime_error("Could not find a suitable physical device to use.");
  }

  // If we reached here, we found a physical device and are ready to create
  // the logical device, which is a connection to the physical device (usage
  // of Vulkan on a certain GPU). The first struct is used to indicate what
  // queues we want to create along with the device. We pass an array of these
  // structs, and if we want to create queues of different families the array
  // contains one element for each family.
  vector<VkDeviceQueueCreateInfo> queue_infos;
  vector<float> queue_priorities = { 1.0f };

  VkDeviceQueueCreateInfo queue_info = {};
  queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queue_info.queueFamilyIndex = graphics_family;
  queue_info.queueCount = 1;
  queue_info.pQueuePriorities = queue_priorities.data();

  queue_infos.push_back(queue_info);

  // One create info for compute queue
  if (graphics_family != compute_family) {
    queue_info.queueFamilyIndex = compute_family;
    queue_infos.push_back(queue_info);
  }

  // An additional create info if graphics queue family is different from
  // the present queue family.
  if (graphics_family != present_family) {
    queue_info.queueFamilyIndex = present_family;
    queue_infos.push_back(queue_info);
  }

  // Features are additional hardware capabilities that are disabled by
  // default. If we want, we could pass a structure that was filled when
  // calling vkGetPhysicalDeviceFeatures, which is the easiest way to enable
  // all supported features.
  VkPhysicalDeviceFeatures features = {};
  features.shaderClipDistance = VK_TRUE; // I think this might have something to do with default values for GLSL vertex shader outputs that fucks up in Vulkan
  features.shaderSampledImageArrayDynamicIndexing = VK_TRUE;
  features.textureCompressionBC = VK_TRUE;
  features.geometryShader = VK_TRUE;

  VkDeviceCreateInfo device_info = {};
  device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  device_info.queueCreateInfoCount = static_cast<uint32_t>(queue_infos.size());
  device_info.pQueueCreateInfos = queue_infos.data();
  device_info.enabledExtensionCount = static_cast<uint32_t>(extensions_.size());
  device_info.ppEnabledExtensionNames = extensions_.data();
  device_info.pEnabledFeatures = &features;

  VkDevice device = VK_NULL_HANDLE;
  VkResult result = physical_device->vkCreateDevice(&device_info, nullptr, &device);
  if (result != VK_SUCCESS) {
    throw runtime_error("Could not create Vulkan device.");
  }

  auto device_wrap = unique_ptr<Device>(new Device(device, physical_device, instance));
  device_wrap->graphics_family_ = graphics_family;
  device_wrap->compute_family_ = compute_family;
  device_wrap->present_family_ = present_family;

  VkQueue queue;
  device_wrap->vkGetDeviceQueue(graphics_family, 0, &queue);
  device_wrap->graphics_queue_ = shared_ptr<Queue>(new Queue(queue, device_wrap.get()));
  device_wrap->vkGetDeviceQueue(compute_family, 0, &queue);
  device_wrap->compute_queue_ = shared_ptr<Queue>(new Queue(queue, device_wrap.get()));
  device_wrap->vkGetDeviceQueue(present_family, 0, &queue);
  device_wrap->present_queue_ = shared_ptr<Queue>(new Queue(queue, device_wrap.get()));

  return device_wrap;
}

bool DeviceBuilder::is_physical_device_suitable(PhysicalDevice* physical_device, uint32_t& graphics_family, uint32_t& compute_family, uint32_t& present_family) {
  // Find queue(s) that support the types of operations we are interested in.
  uint32_t queue_family_count = 0;
  physical_device->vkGetPhysicalDeviceQueueFamilyProperties(&queue_family_count, nullptr);
  vector<VkQueueFamilyProperties> queue_family_properties(queue_family_count);
  physical_device->vkGetPhysicalDeviceQueueFamilyProperties(&queue_family_count, queue_family_properties.data());

  // There is no guarantee that there exists a queue family with support for
  // both graphics and present, so these families are tracked independently.
  // However, they may very well be the same (we strive for that), in which
  // case the indices just happen to be the same.
  uint32_t selected_graphics_family = UINT32_MAX;
  uint32_t selected_compute_family = UINT32_MAX;
  uint32_t selected_present_family = UINT32_MAX;

  // Find graphics and present queues, prefering when they're equal.
  for (uint32_t i = 0; i < queue_family_count; ++i) {
    // Check whether this particular queue family of the given physical
    // device supports presentation.
#if defined(_WIN32)
    if (physical_device->vkGetPhysicalDeviceWin32PresentationSupportKHR(i) == VK_TRUE) {
      selected_present_family = i;
    }
#else
#error "Unsupported OS"
#endif

    // Graphics operations support
    if (queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      selected_graphics_family = i;

      // We prefer if the queue family also supports presentation
      if (selected_present_family == i) {
        break; // Device suitable for graphics/presentation
      }
    }
  }

  // Find a family with compute capabilities, preferably without graphics
  // (for HW with async compute).
  for (uint32_t i = 0; i < queue_family_count; ++i) {
    if (queue_family_properties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
      selected_compute_family = i;
      if (!(queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
        break;
      }
    }
  }

  // If any family is unsupported we must try another device.
  if (selected_graphics_family == UINT32_MAX || selected_compute_family == UINT32_MAX || selected_present_family == UINT32_MAX) {
    return false;
  }

  // If we reached here the device is suitable so we use it and the queue
  // families calculated.
  graphics_family = selected_graphics_family;
  compute_family = selected_compute_family;
  present_family = selected_present_family;

  return true;
};

}
