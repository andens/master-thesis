#include "config-setter.h"

#include <algorithm>
#include <array>
#include "../scene/scene.h"

bool ConfigSetter::more() const {
  return next_config_ != configurations_.end();
}

void ConfigSetter::next_config(Scene& scene) {
  if (more()) {
    // do stuff
  }
}

ConfigSetter::ConfigSetter(uint32_t max_draw_calls) {
  // Create the configurations we will use
  std::array<Renderer::RenderStrategy, 3> strategies {
    Renderer::RenderStrategy::Regular,
    Renderer::RenderStrategy::MDI,
    Renderer::RenderStrategy::DGC,
  };

  std::for_each(strategies.begin(), strategies.end(), [this, max_draw_calls](Renderer::RenderStrategy s) {
    for (int pipeline_switches = 1; pipeline_switches <= max_draw_calls; pipeline_switches += 1000) {
      static bool first = true;
      static bool second = false;

      if (second) {
        pipeline_switches--; // Should be multiples of 1000 from now on
      }

      for (int update_ratio = 1; update_ratio <= 100; ++update_ratio) {
        Configuration c {};
        c.strategy = s;
        c.num_pipeline_commands = pipeline_switches;
        c.update_ratio = update_ratio;
        configurations_.push_back(c);
      }

      if (first) {
        first = false;
        second = true;
      }
    }
  });

  next_config_ = configurations_.begin();
}

uint32_t ConfigSetter::num_configurations() const {
  return static_cast<uint32_t>(configurations_.size());
}
