#pragma once

#include <cstdint>
#include <memory>

class Camera;
class GLFWcursor;
class GLFWwindow;
class Renderer;
class Scene;

class App {
public:
  void run();

  App();
  ~App();

private:
  void frame(float delta_time, float total_time);
  void key_callback(int key, int scancode, int action, int mods);

private:
  GLFWwindow* window_ { nullptr };
  GLFWcursor** mouse_cursors_ { nullptr };
  uint32_t window_width_ { 1280 };
  uint32_t window_height_ { 720 };
  std::unique_ptr<Renderer> renderer_;
  std::unique_ptr<Camera> camera_;
  std::unique_ptr<Scene> scene_;
};
