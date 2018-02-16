#pragma once

#include <memory>
#include <vulkan-helpers/vulkan_include.inl>

namespace graphics {
class DepthBuffer;

namespace deferred_shading {
class GBuffer;
}
}

namespace vkgen {
class GlobalFunctions;
}

namespace vk {
class CommandBuffer;
class CommandPool;
class Device;
class Instance;
class PresentationSurface;
class Queue;
class Swapchain;
}

class Renderer {
public:
  Renderer(HWND hwnd, uint32_t render_width, uint32_t render_height);
  ~Renderer();

private:
  void create_instance();
  void create_debug_callback();
  void create_surface(HWND hwnd);
  void create_device();
  void create_swapchain();
  void create_command_pools_and_buffers();
  void create_render_pass();
  void create_framebuffer();
  void create_shaders();
  void create_pipeline();
  void create_synchronization_primitives();
  void configure_barrier_structs();

private:
  VkExtent2D render_area_ { 0, 0 };
  std::unique_ptr<vkgen::GlobalFunctions> vk_globals_;
  std::shared_ptr<vk::Instance> instance_;
  VkDebugReportCallbackEXT debug_callback_ { VK_NULL_HANDLE };
  std::shared_ptr<vk::PresentationSurface> surface_;
  std::shared_ptr<vk::Device> device_;
  std::shared_ptr<vk::Queue> graphics_queue_;
  std::shared_ptr<vk::Queue> compute_queue_;
  std::shared_ptr<vk::Queue> present_queue_;
  std::unique_ptr<vk::Swapchain> swapchain_;
  std::shared_ptr<vk::CommandPool> stable_graphics_cmd_pool_;
  std::shared_ptr<vk::CommandPool> transient_graphics_cmd_pool_;
  std::shared_ptr<vk::CommandBuffer> graphics_cmd_buf_;
  std::shared_ptr<vk::CommandBuffer> blit_swapchain_cmd_buf_;
  std::unique_ptr<graphics::DepthBuffer> depth_buffer_;
  std::unique_ptr<graphics::deferred_shading::GBuffer> gbuffer_;
  VkRenderPass gbuffer_render_pass_ { VK_NULL_HANDLE };
  VkFramebuffer framebuffer_ { VK_NULL_HANDLE };
  VkShaderModule fullscreen_triangle_vs_ { VK_NULL_HANDLE };
  VkShaderModule fill_gbuffer_vs_ { VK_NULL_HANDLE };
  VkShaderModule fill_gbuffer_fs_ { VK_NULL_HANDLE };
  VkPipelineLayout gbuffer_pipeline_layout_ { VK_NULL_HANDLE };
  VkPipeline gbuffer_pipeline_ { VK_NULL_HANDLE };
  VkSemaphore image_available_semaphore_ { VK_NULL_HANDLE };
  VkSemaphore blit_swapchain_complete_ { VK_NULL_HANDLE };
  VkFence render_fence_ { VK_NULL_HANDLE };
  VkImageMemoryBarrier present_to_transfer_barrier_ {};
  VkImageMemoryBarrier transfer_to_present_barrier_ {};
};
