#include "render-jobs-descriptor-set.h"

#include <vulkan-helpers/buffer.h>
#include <vulkan-helpers/descriptor_pool.h>
#include <vulkan-helpers/descriptor_set.h>
#include <vulkan-helpers/descriptor_set_layout.h>
#include <vulkan-helpers/descriptor_set_layout_builder.h>
#include <vulkan-helpers/device.h>

const uint32_t max_render_jobs = 10000;

void RenderJobsDescriptorSet::destroy(vk::Device& device) {
  layout_->destroy(device);
  storage_buffer_->destroy(device);
}

vk::DescriptorSetLayout const& RenderJobsDescriptorSet::layout() const {
  return *layout_;
}

vk::DescriptorSet const& RenderJobsDescriptorSet::set() const {
  return *set_;
}

void RenderJobsDescriptorSet::update_data(vk::Device& device, uint32_t render_job, std::function<void(RenderJobData&)> const& update) {
  // Note: This implies that the entire render job has to be updated at once.
  RenderJobData data {};
  update(data);

  void* mapped_data = nullptr;
  device.vkMapMemory(storage_buffer_->vulkan_memory_handle(), render_job * sizeof(RenderJobData), sizeof(RenderJobData), 0, &mapped_data);
  memcpy(mapped_data, &data, sizeof(RenderJobData));

  VkMappedMemoryRange flush_range {};
  flush_range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
  flush_range.pNext = nullptr;
  flush_range.memory = storage_buffer_->vulkan_memory_handle();
  flush_range.offset = render_job * sizeof(RenderJobData);
  flush_range.size = sizeof(RenderJobData);
  device.vkFlushMappedMemoryRanges(1, &flush_range);

  device.vkUnmapMemory(storage_buffer_->vulkan_memory_handle());
}

RenderJobsDescriptorSet::RenderJobsDescriptorSet(vk::Device& device, vk::DescriptorPool& pool) {
  storage_buffer_.reset(new vk::Buffer { device, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, sizeof(RenderJobData) * max_render_jobs });

  vk::DescriptorSetLayoutBuilder layout_builder;
  layout_builder.storage_buffer(VK_SHADER_STAGE_VERTEX_BIT, 1);
  layout_.reset(new vk::DescriptorSetLayout { device, layout_builder });
  set_ = pool.allocate(device, *layout_);

  set_->write_storage_buffer(0, storage_buffer_->vulkan_buffer_handle(), 0, sizeof(RenderJobData) * max_render_jobs);
  set_->update_pending(device);
}

RenderJobsDescriptorSet::~RenderJobsDescriptorSet() {}
