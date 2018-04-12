#include "scene.h"

#include <algorithm>
#include <array>
#include <imgui.h>
#include "../renderer/renderer.h"
#include "../render-cache/render-cache.h"

void Scene::update(float delta_time, Renderer& renderer) {
  // Get the previous frame render time to update graph
  accumulated_timings_.total += renderer.measured_time();
  accumulated_timings_.gpu += renderer.gpu_time();
  accumulated_timings_.dgc_generation += renderer.dgc_generation_time();
  accumulated_timings_.traversal += renderer.render_jobs_traversal_time();

  accumulated_frames_++;
  accumulation_timer_ += delta_time;
  if (accumulation_timer_ > time_per_accumulation_) {
    accumulation_timer_ -= time_per_accumulation_;
    render_time_history_.pop_front();
    accumulated_timings_.total /= accumulated_frames_; // Average
    accumulated_timings_.gpu /= accumulated_frames_; // Average
    accumulated_timings_.dgc_generation /= accumulated_frames_; // Average
    accumulated_timings_.traversal /= accumulated_frames_; // Average
    render_time_history_.push_back(accumulated_timings_);
    accumulated_timings_ = {}; // Reset
    accumulated_frames_ = 0;
  }

  if (measuring_) {
    next_measured_frame(renderer);
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
      largest_history_entry_ = 0.0;
      //ImVec2 plot_extent { ImGui::GetContentRegionAvailWidth() - 70, 100 };
      ImVec2 plot_extent { ImGui::GetContentRegionAvailWidth(), 100 };
      ImGui::PlotLines("", [](void* data, int idx) -> float {
        auto self = reinterpret_cast<Scene*>(data);
        double entry = self->monitor_value_(self->render_time_history_[idx]);
        if (entry > self->largest_history_entry_) {
          self->largest_history_entry_ = entry;
        }
        return static_cast<float>(entry);
      }, this, render_time_history_.size(), 0, nullptr, 0.0f, FLT_MAX, plot_extent);

      ImGui::Columns(2);

      if (ImGui::RadioButton("Total", monitor_variant_ == MonitorVariant::Total)) {
        set_monitor_variant(MonitorVariant::Total);
      }
      ImGui::SameLine();
      ImGui::Text("(?)");
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Complete time from recording on CPU until rendering finished.");
      }

      if (ImGui::RadioButton("GPU", monitor_variant_ == MonitorVariant::Gpu)) {
        set_monitor_variant(MonitorVariant::Gpu);
      }
      ImGui::SameLine();
      ImGui::Text("(?)");
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("GPU time spent rendering the scene and UI.\nNote: Includes DGC generation.");
      }

      if (ImGui::RadioButton("CPU", monitor_variant_ == MonitorVariant::Cpu)) {
        set_monitor_variant(MonitorVariant::Cpu);
      }
      ImGui::SameLine();
      ImGui::Text("(?)");
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("CPU time, including traversing jobs.");
      }

      if (ImGui::RadioButton("Traverse", monitor_variant_ == MonitorVariant::Traverse)) {
        set_monitor_variant(MonitorVariant::Traverse);
      }
      ImGui::SameLine();
      ImGui::Text("(?)");
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Traverse render jobs without doing anything with them.");
      }

      if (ImGui::RadioButton("DGC gen.", monitor_variant_ == MonitorVariant::DgcGen)) {
        set_monitor_variant(MonitorVariant::DgcGen);
      }
      ImGui::SameLine();
      ImGui::Text("(?)");
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("GPU time spent generating commands.\nDoes not include executing them.");
      }

      if (ImGui::RadioButton("Overhead", monitor_variant_ == MonitorVariant::Overhead)) {
        set_monitor_variant(MonitorVariant::Overhead);
      }
      ImGui::SameLine();
      ImGui::Text("(?)");
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Overhead incurred by a strategy. This is the time spent with\nrecording, updating indirect buffers, and DGC generation.\nCalculated as total - traversal - gpu + dgc gen.");
      }

      ImGui::NextColumn();

      ImGui::Text("Max: %-3.4f ms", largest_history_entry_);
      ImGui::Text("Latest: %-3.4f ms", monitor_value_(render_time_history_.back()));
    }
  }
  ImGui::End();

  if (ImGui::Begin("Controls")) {
    if (measuring_) {
      ImGui::Text("Measuring in progress - please hold. Frame %u of %u.", measure_current_frame_, measure_frame_span_);
    }
    else {
      typedef std::pair<char const*, Renderer::RenderStrategy> Strategy;
      std::array<Strategy, 3> strategies {
        std::make_pair("Regular", Renderer::RenderStrategy::Regular),
        std::make_pair("MDI", Renderer::RenderStrategy::MDI),
        std::make_pair("DGC", Renderer::RenderStrategy::DGC),
      };

      Renderer::RenderStrategy current_strategy = renderer.current_strategy();

      auto strategy = std::find_if(strategies.begin(), strategies.end(), [current_strategy](Strategy const& s) -> bool {
        return current_strategy == s.second;
      });

      if (ImGui::BeginCombo("Strategy", strategy->first, 0)) {
        std::for_each(strategies.begin(), strategies.end(), [current_strategy, &renderer](Strategy const& s) {
          bool selected = current_strategy == s.second;
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
        if (ImGui::DragInt("##pipeline switches", &pipeline_switches_, 100.0f, 1, max_draw_calls_)) {
          if (pipeline_switches_ % 100 == 1 && pipeline_switches_ != 1) {
            pipeline_switches_--;
          }
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

      // Number of updated jobs per frame
      {
        ImGui::AlignFirstTextHeightToWidgets();
        char const* left_label = "Update";
        ImGui::Text(left_label);
        ImGui::SameLine();
        ImGui::PushItemWidth(100.0f);
        if (ImGui::SliderInt("##update_ratio", &update_ratio_, 0, 100, "%.0f%%"));
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::Text("of jobs each frame");
        ImGui::SameLine();
        ImGui::Text("(?)");
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Frequency of incremental changes during a frame.\nFor MDI and DGC, this is how many draw calls are\nrecorded during the frame. Regular is unaffected\nby this value as everything is always recorded.");
        }
        ImGui::SameLine();
        ImGui::Text("(%u / %u).", max_draw_calls_ / 100 * update_ratio_, max_draw_calls_);
      }

      // Start measuring session
      {
        if (ImGui::Button("Measure")) {
          measuring_ = true;
        }
      }
    }
  }
  ImGui::End();

  renderer.borrow_render_cache([this](RenderCache& cache) {
    uint32_t dirty_count = max_draw_calls_ / 100 * update_ratio_;
    for (uint32_t i = 0; i < dirty_count; ++i) {
      cache.dirtify(i);
    }
  });
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

  set_monitor_variant(MonitorVariant::Total);

  uint32_t entries = history_time_span_ / time_per_accumulation_;
  for (uint32_t i = 0; i < entries; ++i) {
    FrameTimings timings {};
    render_time_history_.push_back(timings);
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

void Scene::next_measured_frame(Renderer& r) {
  measure_timings_.total += r.measured_time();
  measure_timings_.gpu += r.gpu_time();
  measure_timings_.dgc_generation += r.dgc_generation_time();
  measure_timings_.traversal += r.render_jobs_traversal_time();

  if (measure_current_frame_ == measure_frame_span_) {
    finish_measure_session();
  }
  else {
    measure_current_frame_++;
  }
}

void Scene::finish_measure_session() {
  // Average the times
  measure_timings_.total /= measure_frame_span_;
  measure_timings_.gpu /= measure_frame_span_;
  measure_timings_.dgc_generation /= measure_frame_span_;
  measure_timings_.traversal /= measure_frame_span_;

  // TODO: push timings and config

  // Reset for next measure session
  measure_timings_ = {};
  measure_current_frame_ = 0;
  measuring_ = false;
}

void Scene::set_monitor_variant(MonitorVariant v) {
  monitor_variant_ = v;

  switch (monitor_variant_) {
  case MonitorVariant::Total: {
    monitor_value_ = [](FrameTimings const& t) -> double { return t.total; };
    break;
  }
  case MonitorVariant::Gpu: {
    monitor_value_ = [](FrameTimings const& t) -> double { return t.gpu; };
    break;
  }
  case MonitorVariant::Cpu: {
    monitor_value_ = [](FrameTimings const& t) -> double { return t.total - t.gpu; };
    break;
  }
  case MonitorVariant::Traverse: {
    monitor_value_ = [](FrameTimings const& t) -> double { return t.traversal; };
    break;
  }
  case MonitorVariant::DgcGen: {
    monitor_value_ = [](FrameTimings const& t) -> double { return t.dgc_generation; };
    break;
  }
  case MonitorVariant::Overhead: {
    monitor_value_ = [](FrameTimings const& t) -> double { return t.total - t.traversal - t.gpu + t.dgc_generation; };
    break;
  }
  default: throw;
  }
}
