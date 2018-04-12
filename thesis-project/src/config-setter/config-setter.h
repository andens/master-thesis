#pragma once

class Scene;

class ConfigSetter {
public:
  bool more() const;
  void next_config(Scene& scene);
};
