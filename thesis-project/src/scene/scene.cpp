#include "scene.h"

#include <algorithm>
#include <imgui.h>
#include "../renderer/renderer.h"
#include "../render-cache/render-cache.h"

void Scene::update(float delta_time, Renderer& renderer) {
  // Get the previous frame render time to update graph
  accumulated_render_time_ += renderer.measured_time();
  accumulated_frames_++;
  accumulation_timer_ += delta_time;
  if (accumulation_timer_ > time_per_accumulation_) {
    accumulation_timer_ -= time_per_accumulation_;
    render_time_history_.pop_front();
    render_time_history_.push_back(static_cast<float>(accumulated_render_time_ / accumulated_frames_));
    accumulated_render_time_ = 0.0;
    accumulated_frames_ = 0;
    largest_history_entry_ = 0.0f;
    std::for_each(render_time_history_.begin(), render_time_history_.end(), [this](float val) {
      if (val > largest_history_entry_) {
        largest_history_entry_ = val;
      }
    });
  }

  // Simple logic to stop and start rendering the first object in intervals
  // of one second. Every new reappearance switches mesh to demonstrate that
  // draw calls can not only be individually updated, they can also completely
  // alter what is rendered.
  /*
  static float time = 0.0f;
  static bool stop_rendering = false;
  static bool box = true;
  time += delta_time;
  if (time > 1.0f) {
    time = 0.0f;
    stop_rendering = !stop_rendering;
    renderer.borrow_render_cache([](RenderCache& cache) {
      if (stop_rendering) {
        cache.stop_rendering(0);
      }
      else {
        box = !box;
        cache.start_rendering(0, box ? RenderObject::Box : RenderObject::Sphere, RenderCache::Pipeline::Alpha);
      }
    });
  }
  */

  if (ImGui::Begin("Controls")) {
    if (ImGui::CollapsingHeader("Render time", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen)) {
      ImVec2 plot_extent { ImGui::GetContentRegionAvailWidth() - 70, 100 };
      ImGui::PlotLines("", [](void* data, int idx) -> float {
        auto frame_times = reinterpret_cast<std::deque<float>*>(data);
        return (*frame_times)[idx];
      }, &render_time_history_, render_time_history_.size(), 0, nullptr, 0.0f, FLT_MAX, plot_extent);

      ImGui::SameLine();

      ImGui::Text("%-3.4f ms", largest_history_entry_);
    }
  }
  ImGui::End();
}

Scene::Scene(Renderer& renderer) {
  /*
  renderer.update_transform(0, DirectX::XMMatrixTranslation(0.0f, 0.0f, 10.0f));
  renderer.update_transform(1, DirectX::XMMatrixTranslation(5.0f, 0.0f, 10.0f));

  renderer.borrow_render_cache([](RenderCache& cache) {
    cache.start_rendering(0, RenderObject::Box, RenderCache::Pipeline::Alpha);
    cache.start_rendering(1, RenderObject::Sphere, RenderCache::Pipeline::Alpha);
  });
  */

  uint32_t entries = history_time_span_ / time_per_accumulation_;
  for (uint32_t i = 0; i < entries; ++i) {
    render_time_history_.push_back(0.0f);
  }
}
