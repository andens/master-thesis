#pragma once

#include <memory>
#include "vulkan_include.inl"

namespace vk {

class Device;
class CommandBuffer;

class CommandPool {
public:
  static std::unique_ptr<CommandPool> make_stable(uint32_t queue_family, std::weak_ptr<Device> device);
  static std::unique_ptr<CommandPool> make_transient(uint32_t queue_family, std::weak_ptr<Device> device);

  std::unique_ptr<CommandBuffer> allocate_primary();
  std::unique_ptr<CommandBuffer> allocate_secondary();

  ~CommandPool();

private:
  CommandPool() {}
  CommandPool(CommandPool& other) = delete;
  void operator=(CommandPool& rhs) = delete;

  static std::unique_ptr<CommandPool> make(VkCommandPoolCreateInfo const& pool_info, std::weak_ptr<Device> device);

private:
  VkCommandPool pool_ = VK_NULL_HANDLE;
  std::weak_ptr<Device> device_;
};

}
