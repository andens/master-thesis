#include "app.h"

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <iostream>
#include <stdexcept>
#include "renderer/renderer.h"

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

  glfwSetWindowUserPointer(window_, this);

  glfwSetKeyCallback(window_, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
    auto app = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));
    app->key_callback(key, scancode, action, mods);
  });

  renderer_.reset(new Renderer(glfwGetWin32Window(window_)));

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

  renderer_.reset(nullptr);

  std::cin.get();
}

App::App() = default;
App::~App() = default;

void App::frame(float delta_time, float total_time) {

}

void App::key_callback(int key, int scancode, int action, int mods) {
  if (key == GLFW_KEY_ESCAPE) {
    glfwSetWindowShouldClose(window_, GLFW_TRUE);
  }
}
