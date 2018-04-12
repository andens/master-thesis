#pragma once

#include <vector>
#include "../renderer/renderer.h"

class Scene;

class ConfigSetter {
public:
  bool more() const;
  void next_config(Scene& scene);
  uint32_t num_configurations() const;

  ConfigSetter(uint32_t max_draw_calls);

private:
  struct Configuration {
    Renderer::RenderStrategy strategy;
    int num_pipeline_commands;
    int update_ratio;
  };

private:
  std::vector<Configuration> configurations_ {};
  std::vector<Configuration>::iterator next_config_ {};
};
