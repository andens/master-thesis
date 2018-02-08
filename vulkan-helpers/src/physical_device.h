#pragma once

#include <vector>
#include "vk_dispatch_tables.h"

namespace vk {

class PhysicalDevice : public vkgen::PhysicalDeviceFunctions {
public:
  PhysicalDevice(VkPhysicalDevice phys_dev, vkgen::InstanceFunctions* instance);

  const VkPhysicalDeviceMemoryProperties& memory_properties(void);
  std::vector<VkSurfaceFormatKHR> supported_surface_formats(VkSurfaceKHR surface);
  std::vector<VkPresentModeKHR> supported_present_modes(VkSurfaceKHR surface);
  // Find a supported format among a prioritized list of formats.
  VkFormat find_supported_format(std::vector<VkFormat> const& desirables, VkImageTiling tiling, VkFormatFeatureFlags features);

private:
  VkPhysicalDeviceMemoryProperties memory_properties_ = {};
};

}
