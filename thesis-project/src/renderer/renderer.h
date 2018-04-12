#pragma once

#include <DirectXMath.h>
#include <functional>
#include <memory>
#include <vector>
#include <vulkan-helpers/vulkan_include.inl>

class GuiDescriptorSet;
class ImDrawData;
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
  enum class RenderStrategy {
    Regular,
    MDI,
    DGC,
  };

public:
  Renderer(HWND hwnd, uint32_t render_width, uint32_t render_height);
  ~Renderer();

  void render();
  void use_matrices(DirectX::CXMMATRIX view, DirectX::CXMMATRIX proj);
  void borrow_render_cache(std::function<void(RenderCache& cache)> const& provide);
  void update_transform(uint32_t render_job, DirectX::CXMMATRIX transform);
  void should_render_ui(bool should);
  // Returns the measured time from starting rendering (including recording
  // commands normally or incrementally) until it's finished, stopping before
  // blitting to swapchain and presenting. For fair measurements, the GUI
  // should also be disabled.
  double measured_time() const;
  // Returns the time spent on the GPU. CPU overhead is thus the difference
  // between total time and GPU time, provided that the GPU does not start
  // processing until the CPU work is completely done.
  double gpu_time() const;
  // Returns the time it takes DGC to patch the command buffer on the GPU.
  // This time is part of the overhead of that strategy and should not be
  // ignored in the total GPU time.
  double dgc_generation_time() const;
  // Returns the time spent traversing the render jobs data structure. This is
  // overhead that is present for each technique and can be subtracted to get
  // an idea of how the render strategy itself performs.
  double render_jobs_traversal_time() const;

  void use_render_strategy(RenderStrategy strategy);
  RenderStrategy current_strategy() const;

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
  void generate_triangles(std::vector<Vertex>& vertices);
  void interleave_vertex_data(Mesh* mesh, std::vector<Vertex>& vertices);
  void create_descriptor_sets();
  void create_indirect_buffer();
  void update_indirect_buffer(std::function<void(uint32_t job, VkPipeline pipeline)> const& pipeline_switch, bool dgc);
  void initialize_imgui();
  void create_imgui_font_texture();
  void update_gui_vertex_data(ImDrawData* draw_data);
  void create_dgc_resources();
  void create_object_table();
  void register_objects_in_table();
  void create_indirect_commands_layout();
  void reserve_space_for_indirect_commands();
  void create_timing_resources();

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
  VkPhysicalDeviceProperties physical_device_properties_;
  std::unique_ptr<vk::Swapchain> swapchain_;
  std::shared_ptr<vk::CommandPool> stable_graphics_cmd_pool_;
  std::shared_ptr<vk::CommandPool> transient_graphics_cmd_pool_;
  std::shared_ptr<vk::CommandBuffer> graphics_cmd_buf_;
  std::shared_ptr<vk::CommandBuffer> dgc_cmd_buf_;
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
  VkPipeline pipeline_regular_mdi_wireframe_ { VK_NULL_HANDLE };
  VkPipeline pipeline_regular_mdi_solid_ { VK_NULL_HANDLE };
  VkPipeline pipeline_dgc_wireframe_ { VK_NULL_HANDLE };
  VkPipeline pipeline_dgc_solid_ { VK_NULL_HANDLE };
  VkPipeline gui_pipeline_{ VK_NULL_HANDLE };
  VkSemaphore image_available_semaphore_ { VK_NULL_HANDLE };
  VkSemaphore blit_swapchain_complete_ { VK_NULL_HANDLE };
  VkSemaphore gbuffer_generation_complete_ { VK_NULL_HANDLE };
  VkFence render_fence_ { VK_NULL_HANDLE };
  VkFence gbuffer_generation_fence_ { VK_NULL_HANDLE };
  VkImageMemoryBarrier present_to_transfer_barrier_ {};
  VkImageMemoryBarrier transfer_to_present_barrier_ {};
  VkSampler gui_font_sampler_ { VK_NULL_HANDLE };
  VkImage gui_font_image_ { VK_NULL_HANDLE };
  VkDeviceMemory gui_font_image_memory_ { VK_NULL_HANDLE };
  VkImageView gui_font_image_view_ { VK_NULL_HANDLE };

  std::unique_ptr<vk::Buffer> vertex_buffer_;
  std::unique_ptr<vk::Buffer> gui_vertex_buffer_;
  std::unique_ptr<vk::Buffer> gui_index_buffer_;
  size_t gui_vertex_buffer_size_ { 0 };
  size_t gui_index_buffer_size_ { 0 };

  std::unique_ptr<vk::DescriptorPool> descriptor_pool_;
  std::unique_ptr<RenderJobsDescriptorSet> render_jobs_descriptor_set_;
  std::unique_ptr<GuiDescriptorSet> gui_descriptor_set_;

  std::unique_ptr<RenderCache> render_cache_;

  // Note: My draw call management is hardcoded to two different pipelines
  // because it eases implementation as I can manage the indirect buffer as a
  // double-ended stack. This essentially corresponds to two buckets of dynamic
  // size. In a real world scenario one may want more elaborate infrastructure
  // that tracks an arbitrary amount of (likely equisized) buckets.
  VkDrawIndirectCommand* mapped_indirect_buffer_ { nullptr };
  std::unique_ptr<vk::Buffer> indirect_buffer_;
  const uint32_t max_draw_calls_ { 100000 };

  VkObjectTableNVX object_table_ { VK_NULL_HANDLE };
  VkIndirectCommandsLayoutNVX indirect_commands_layout_ { VK_NULL_HANDLE };
  std::unique_ptr<vk::Buffer> dgc_pipeline_parameters_;
  uint32_t* mapped_dgc_pipeline_parameters_ { nullptr };
  std::unique_ptr<vk::Buffer> dgc_push_constants_;
  struct Push {
    uint32_t table_entry;
    uint32_t actual_data;
  };
  Push* mapped_dgc_push_constants_ { nullptr };

  VkQueryPool query_pool_ { VK_NULL_HANDLE };

  DirectX::XMFLOAT4X4 view_ {};
  DirectX::XMFLOAT4X4 proj_ {};

  double measured_time_ { 0.0 };
  double gpu_render_time_ { 0.0 };
  double dgc_generation_time_ { 0.0 };
  double render_jobs_traversal_time_ { 0.0 };

  bool measure_session_active_ { false };
  RenderStrategy render_strategy_ { RenderStrategy::Regular };
};
