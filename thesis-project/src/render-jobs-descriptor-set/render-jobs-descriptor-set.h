#pragma once

#include <DirectXMath.h>
#include <functional>
#include <memory>

namespace vk {
class Buffer;
class DescriptorPool;
class DescriptorSet;
class DescriptorSetLayout;
class Device;
}

class RenderJobsDescriptorSet {
public:
  struct RenderJobData {
    DirectX::XMFLOAT4X4 transform;
  };

public:
  void destroy(vk::Device& device);
  vk::DescriptorSetLayout const& layout() const;
  vk::DescriptorSet const& set() const;
  void update_data(vk::Device& device, uint32_t render_job, std::function<void(RenderJobData&)> const& update);

  RenderJobsDescriptorSet(vk::Device& device, vk::DescriptorPool& pool);
  ~RenderJobsDescriptorSet();

private:
  RenderJobsDescriptorSet(RenderJobsDescriptorSet& other) = delete;
  void operator=(RenderJobsDescriptorSet& rhs) = delete;

private:
  std::unique_ptr<vk::Buffer> storage_buffer_;
  std::unique_ptr<vk::DescriptorSetLayout> layout_;
  std::unique_ptr<vk::DescriptorSet> set_;
};
