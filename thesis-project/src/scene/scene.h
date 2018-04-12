#pragma once

#include <deque>
#include <functional>
#include <vector>
#include "../renderer/renderer.h"

class ConfigSetter;

class Scene {
public:
  void update(float delta_time, Renderer& renderer);

  Scene(Renderer& renderer);
  ~Scene();

private:
  struct FrameTimings {
    double total;
    double gpu;
    double dgc_generation;
    double traversal;
  };

  enum class MonitorVariant {
    Total = 0,
    Gpu,
    Cpu,
    Traverse,
    DgcGen,
    Overhead,
  };

  struct MeasureSession {
    char const* strategy;
    uint32_t num_pipeline_commands;
    uint32_t updates_per_frame;
    FrameTimings timing;
  };

private:
  void modify_pipeline_switch_frequency(Renderer& r);
  void start_measure_suite(); // Run a bunch of sessions with pre-defined configs
  void start_measure_session(Renderer::RenderStrategy current_strategy);
  void next_measured_frame(Renderer& r);
  void finish_measure_session();
  void finish_measure_suite();
  void set_monitor_variant(MonitorVariant v);

private:
  const uint32_t max_draw_calls_ { 100000 };

  FrameTimings accumulated_timings_ {};
  uint32_t accumulated_frames_ { 0 };
  float accumulation_timer_ { 0.0f };
  const float time_per_accumulation_ { 0.1f };

  std::deque<FrameTimings> render_time_history_;
  const float history_time_span_ { 30.0f }; // Seconds of history shown
  double largest_history_entry_ { 0.0f };
  MonitorVariant monitor_variant_ { MonitorVariant::Total };
  std::function<double(FrameTimings const&)> monitor_value_ { nullptr };

  int pipeline_switches_ { 1 };
  int update_ratio_ { 0 };

  bool measuring_ { false };
  bool measuring_suite_ { false }; // Whether we are measuring the suite
  bool suite_measure_skip_this_frame_ { false };
  uint32_t measure_current_frame_ { 1 };
  const uint32_t measure_frame_span_ { 100 };
  uint32_t current_suite_configuration_ { 0 };

  MeasureSession measure_session {};
  std::vector<MeasureSession> sessions_ {};

  std::unique_ptr<ConfigSetter> config_setter_;
};
