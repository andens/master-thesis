#pragma once

#include <cstdint>

class GLFWwindow;

class App {
public:
  void run();

private:
  void frame(float delta_time, float total_time);
  void key_callback(int key, int scancode, int action, int mods);

private:
  GLFWwindow* window_ { nullptr };
  uint32_t window_width_ { 1280 };
  uint32_t window_height_ { 720 };
};
