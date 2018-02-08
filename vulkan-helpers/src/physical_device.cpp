#include "physical_device.h"

using namespace std;

namespace vk {

PhysicalDevice::PhysicalDevice(VkPhysicalDevice phys_dev, vkgen::InstanceFunctions* instance) : vkgen::PhysicalDeviceFunctions(phys_dev, instance) {
  vkGetPhysicalDeviceMemoryProperties(&memory_properties_);
}

const VkPhysicalDeviceMemoryProperties& PhysicalDevice::memory_properties(void) {
  return memory_properties_;
}

vector<VkSurfaceFormatKHR> PhysicalDevice::supported_surface_formats(VkSurfaceKHR surface) {
  uint32_t format_count = 0;
  vkGetPhysicalDeviceSurfaceFormatsKHR(surface, &format_count, nullptr);
  vector<VkSurfaceFormatKHR> surface_formats(format_count);
  vkGetPhysicalDeviceSurfaceFormatsKHR(surface, &format_count, surface_formats.data());
  return surface_formats;
}

vector<VkPresentModeKHR> PhysicalDevice::supported_present_modes(VkSurfaceKHR surface) {
  uint32_t mode_count = 0;
  vkGetPhysicalDeviceSurfacePresentModesKHR(surface, &mode_count, nullptr);
  vector<VkPresentModeKHR> present_modes(mode_count);
  vkGetPhysicalDeviceSurfacePresentModesKHR(surface, &mode_count, present_modes.data());
  return present_modes;
}

VkFormat PhysicalDevice::find_supported_format(vector<VkFormat> const& desirables, VkImageTiling tiling, VkFormatFeatureFlags features) {
  for (auto format : desirables) {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(format, &props);

    if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
      return format;
    }
    else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
      return format;
    }
  }

  throw std::runtime_error("Could not find a supported format.");
}

}
