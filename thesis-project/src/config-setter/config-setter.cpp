#include "config-setter.h"

#include <algorithm>
#include <array>

bool ConfigSetter::more() const {
  return next_config_ != configurations_.end();
}

void ConfigSetter::next_config(std::function<void(Configuration const&)> const& impl) {
  if (more()) {
    impl(*next_config_);
    next_config_++;
  }
}

ConfigSetter::ConfigSetter(uint32_t max_draw_calls) {
  // Create the configurations we will use
  std::array<Renderer::RenderStrategy, 3> strategies {
    Renderer::RenderStrategy::Regular,
    Renderer::RenderStrategy::MDI,
    Renderer::RenderStrategy::DGC,
  };

  std::array<Renderer::Flush, 3> flushes {
    Renderer::Flush::Never,
    Renderer::Flush::Once,
    Renderer::Flush::Individual,
  };

  std::for_each(strategies.begin(), strategies.end(), [this, &flushes, max_draw_calls](Renderer::RenderStrategy s) {
    for (auto flush : flushes) {
      // We don't really care about flush behavior for regular rendering. `Never`
      // will suffice just to have something and the rest are ignored.
      if (s == Renderer::RenderStrategy::Regular && flush != Renderer::Flush::Never) {
        continue;
      }

      uint32_t pipeline_step = max_draw_calls / 10;
      if (max_draw_calls % 10 != 0) throw;
      for (int pipeline_switches = 1; pipeline_switches <= max_draw_calls; pipeline_switches += pipeline_step) {
        // Should be multiples of |pipeline_step| from now on
        if (pipeline_switches == pipeline_step + 1) {
          pipeline_switches--;
        }

        for (int update_ratio = 0; update_ratio <= 100; update_ratio += 10) {
          Configuration c {};
          c.strategy = s;
          c.num_pipeline_commands = pipeline_switches;
          c.update_ratio = update_ratio;
          c.flush_behavior = flush;
          configurations_.push_back(c);
        }
      }
    }
  });

  next_config_ = configurations_.begin();
}

uint32_t ConfigSetter::num_configurations() const {
  return static_cast<uint32_t>(configurations_.size());
}

void ConfigSetter::first_config() {
  next_config_ = configurations_.begin();
}
