#pragma once

#include <functional>
#include <vector>
#include "../renderer/renderer.h"

class ConfigSetter {
public:
  struct Configuration {
    Renderer::RenderStrategy strategy;
    int num_pipeline_commands;
    int update_ratio;
    Renderer::Flush flush_behavior;
  };

public:
  bool more() const;
  void next_config(std::function<void(Configuration const&)> const& impl);
  uint32_t num_configurations() const;
  void first_config();

  ConfigSetter(uint32_t max_draw_calls);

private:
  std::vector<Configuration> configurations_ {};
  std::vector<Configuration>::iterator next_config_ {};
};
