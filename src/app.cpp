#include "app.h"

#include <GLFW/glfw3.h>
#include <stdexcept>

void App::run() {
  if (!glfwInit()) {
    throw std::runtime_error("Could not initialize GLFW.");
  }

  // Don't create a context
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  window_ = glfwCreateWindow(window_width_, window_height_, "Master thesis", nullptr, nullptr);
  if (!window_) {
    glfwTerminate();
    throw std::runtime_error("Could not create a window.");
  }

  float time = glfwGetTime();

  while (!glfwWindowShouldClose(window_)) {
    float t = glfwGetTime();
    float delta_time = t - time;
    time = t;

    frame(delta_time, time);

    glfwPollEvents();
  }

  glfwDestroyWindow(window_);
  window_ = nullptr;
  glfwTerminate();
}

void App::frame(float delta_time, float total_time) {

}
