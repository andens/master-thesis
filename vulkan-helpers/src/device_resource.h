#pragma once

namespace vk {

class Device;

class DeviceResource {
public:
  DeviceResource() = default;
  virtual void destroy(Device& device) = 0;

private:
  DeviceResource(DeviceResource& other) = delete;
  void operator=(DeviceResource& rhs) = delete;
};

}
