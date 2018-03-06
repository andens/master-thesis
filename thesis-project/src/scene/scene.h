#pragma once

class Renderer;

class Scene {
public:
  void update(float delta_time, Renderer& renderer);

  Scene(Renderer& renderer);
};
