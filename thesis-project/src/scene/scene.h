#pragma once

#include <deque>

class Renderer;

class Scene {
public:
  void update(float delta_time, Renderer& renderer);

  Scene(Renderer& renderer);

private:
  void modify_pipeline_switch_frequency(Renderer& r);

private:
  const uint32_t max_draw_calls_ { 100000 };

  double accumulated_render_time_ { 0.0 };
  uint32_t accumulated_frames_ { 0 };
  float accumulation_timer_ { 0.0f };
  const float time_per_accumulation_ { 0.25f };

  std::deque<float> render_time_history_;
  const float history_time_span_ { 60.0f }; // Seconds of history shown
  float largest_history_entry_ { 0.0f };

  int pipeline_switches_ { 1 };
};
