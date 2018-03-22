#pragma once

#include <DirectXMath.h>
#include <functional>
#include <memory>
#include <vector>
#include <vulkan-helpers/vulkan_include.inl>

class GuiDescriptorSet;
class Mesh;
class RenderCache;
class RenderJobsDescriptorSet;

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
class Buffer;
class CommandBuffer;
class CommandPool;
class DescriptorPool;
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

  void render();
  void use_matrices(DirectX::CXMMATRIX view, DirectX::CXMMATRIX proj);
  void borrow_render_cache(std::function<void(RenderCache& cache)> const& provide);
  void update_transform(uint32_t render_job, DirectX::CXMMATRIX transform);

private:
  struct Vertex {
    DirectX::XMFLOAT3 pos;
    DirectX::XMFLOAT2 tex;
    DirectX::XMFLOAT3 nor;
  };

private:
  void create_instance();
  void create_debug_callback();
  void create_surface(HWND hwnd);
  void create_device();
  void create_swapchain();
  void create_command_pools_and_buffers();
  void create_samplers();
  void create_render_pass();
  void create_framebuffer();
  void create_shaders();
  void create_pipeline();
  void create_synchronization_primitives();
  void configure_barrier_structs();
  void create_vertex_buffer();
  void generate_box_vertices(std::vector<Vertex>& vertices);
  void load_sphere(std::vector<Vertex>& vertices);
  void interleave_vertex_data(Mesh* mesh, std::vector<Vertex>& vertices);
  void create_descriptor_sets();
  void create_indirect_buffer();
  void update_indirect_buffer();
  void initialize_imgui();
  void create_imgui_font_texture();

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
  VkShaderModule gui_vs_{ VK_NULL_HANDLE };
  VkShaderModule gui_fs_{ VK_NULL_HANDLE };
  VkPipelineLayout gbuffer_pipeline_layout_ { VK_NULL_HANDLE };
  VkPipelineLayout gui_pipeline_layout_{ VK_NULL_HANDLE };
  VkPipeline gbuffer_pipeline_direct_ { VK_NULL_HANDLE };
  VkPipeline gbuffer_pipeline_indirect_ { VK_NULL_HANDLE };
  VkPipeline gui_pipeline_{ VK_NULL_HANDLE };
  VkSemaphore image_available_semaphore_ { VK_NULL_HANDLE };
  VkSemaphore blit_swapchain_complete_ { VK_NULL_HANDLE };
  VkSemaphore gbuffer_generation_complete_ { VK_NULL_HANDLE };
  VkFence render_fence_ { VK_NULL_HANDLE };
  VkFence gbuffer_generation_fence_ { VK_NULL_HANDLE };
  VkImageMemoryBarrier present_to_transfer_barrier_ {};
  VkImageMemoryBarrier transfer_to_present_barrier_ {};
  VkSampler gui_font_sampler_{ VK_NULL_HANDLE };

  std::unique_ptr<vk::Buffer> vertex_buffer_;

  std::unique_ptr<vk::DescriptorPool> descriptor_pool_;
  std::unique_ptr<RenderJobsDescriptorSet> render_jobs_descriptor_set_;
  std::unique_ptr<GuiDescriptorSet> gui_descriptor_set_;

  std::unique_ptr<RenderCache> render_cache_;

  // Note: My draw call management is hardcoded to two different pipelines
  // because it eases implementation as I can manage the indirect buffer as a
  // double-ended stack. This essentially corresponds to two buckets of dynamic
  // size. In a real world scenario one may want more elaborate infrastructure
  // that tracks an arbitrary amount of (likely equisized) buckets.
  bool render_indirectly_ { true };
  VkDrawIndirectCommand* mapped_indirect_buffer_ { nullptr };
  std::unique_ptr<vk::Buffer> indirect_buffer_;
  const uint32_t max_draw_calls_ { 2000 };
  uint32_t current_total_draw_calls_ { 0 };
  uint32_t current_alpha_draw_calls_ { 0 };
  uint32_t current_beta_draw_calls_ { 0 };

  DirectX::XMFLOAT4X4 view_ {};
  DirectX::XMFLOAT4X4 proj_ {};
};
