#pragma once

#include <cstdint>
#include <memory>
#include <vector>

namespace vk {

class Instance;
class PhysicalDevice;
class Device;

class DeviceBuilder {
public:
  //void with_graphics_queues(uint32_t count);
  //void with_compute_queues(uint32_t count);
  //void with_present_queues(uint32_t count);
  void use_extension(char const* extension);
  std::unique_ptr<Device> build(Instance* instance);

private:
  bool is_physical_device_suitable(PhysicalDevice* physical_device, uint32_t& graphics_family, uint32_t& compute_family, uint32_t& present_family);

private:
  //uint32_t graphics_queues_ = 0;
  //uint32_t compute_queues_ = 0;
  //uint32_t present_queues_ = 0;
  std::vector<char const*> extensions_;
};

}
