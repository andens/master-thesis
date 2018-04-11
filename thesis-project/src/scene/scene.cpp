#include "scene.h"

#include <algorithm>
#include <array>
#include <imgui.h>
#include "../renderer/renderer.h"
#include "../render-cache/render-cache.h"

void Scene::update(float delta_time, Renderer& renderer) {
  // Get the previous frame render time to update graph
  accumulated_render_time_ += (renderer.measured_time() - renderer.gpu_time());
  //accumulated_render_time_ += (renderer.measured_time() - renderer.gpu_time() - renderer.render_jobs_traversal_time());
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

  if (ImGui::Begin("Monitor")) {
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

  if (ImGui::Begin("Controls")) {
    typedef std::pair<char const*, Renderer::RenderStrategy> Strategy;
    std::array<Strategy, 3> strategies {
      std::make_pair("Regular", Renderer::RenderStrategy::Regular),
      std::make_pair("MDI", Renderer::RenderStrategy::MDI),
      std::make_pair("DGC", Renderer::RenderStrategy::DGC),
    };

    auto strategy = std::find_if(strategies.begin(), strategies.end(), [&renderer](Strategy const& s) -> bool {
      return renderer.current_strategy() == s.second;
    });

    if (ImGui::BeginCombo("Strategy", strategy->first, 0)) {
      std::for_each(strategies.begin(), strategies.end(), [&renderer](Strategy const& s) {
        bool selected = renderer.current_strategy() == s.second;
        if (ImGui::Selectable(s.first, &selected, 0)) {
          renderer.use_render_strategy(s.second);
        }
        if (selected) {
          ImGui::SetItemDefaultFocus();
        }
      });
      ImGui::EndCombo();
    }

    // Pipeline switch frequency
    {
      ImGui::AlignFirstTextHeightToWidgets();
      char const* left_label = "Set pipeline";
      ImGui::PushItemWidth(ImGui::CalcTextSize(left_label).x);
      ImGui::Text(left_label);
      ImGui::PopItemWidth();
      ImGui::SameLine();
      ImGui::PushItemWidth(70.0f);
      if (ImGui::SliderInt("##pipeline switches", &pipeline_switches_, 1, max_draw_calls_)) {
        modify_pipeline_switch_frequency(renderer);
      }
      ImGui::PopItemWidth();
      ImGui::SameLine();
      ImGui::Text("time(s)");
      ImGui::SameLine();
      ImGui::Text("(?)");
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("How many pipelines to set during a frame. One\npipeline renders everything the same way, and\nmax switches pipeline for every draw call.");
      }
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

void Scene::modify_pipeline_switch_frequency(Renderer& r) {
  r.borrow_render_cache([this](RenderCache& cache) {
    // The number of pipeline switches is the number of groups to partition the
    // calls in (each group uses one pipeline). The partitioning do not always
    // end up in equi-sized groups. Modulo tells us how many groups should have
    // an extra job (none in case the division evens out). I put these groups
    // at the end.

    RenderCache::Pipeline use_pipeline = RenderCache::Pipeline::Alpha;
    RenderCache::Pipeline switch_pipeline = RenderCache::Pipeline::Beta;

    uint32_t batch_size = max_draw_calls_ / pipeline_switches_;
    uint32_t groups_with_extra_job = max_draw_calls_ % pipeline_switches_;

    uint32_t objects_since_switch = 0;
    uint32_t groups_processed = 0;
    for (uint32_t i = 0; i < max_draw_calls_; ++i) {
      cache.modify_pipeline(i, use_pipeline);

      ++objects_since_switch;

      if (objects_since_switch == batch_size) {
        objects_since_switch = 0;
        groups_processed++;
        std::swap(use_pipeline, switch_pipeline);

        if (groups_processed == pipeline_switches_ - groups_with_extra_job) {
          batch_size++;
        }
      }
    }
  });
}
