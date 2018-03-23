#include "app.h"

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <imgui.h>
#include <iostream>
#include <stdexcept>
#include "camera/camera.h"
#include "render-cache/render-cache.h"
#include "renderer/renderer.h"
#include "scene/scene.h"

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

  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();

  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable keyboard controls
  io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors; // Probably means we have access to change OS cursor to match values from imgui?
#if defined(_WIN32)
  io.ImeWindowHandle = glfwGetWin32Window(window_);
#endif

  // Keyboard map. Used by ImGui to index the io.KeyDown array.
  io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;
  io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
  io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
  io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
  io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
  io.KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
  io.KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
  io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
  io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
  io.KeyMap[ImGuiKey_Insert] = GLFW_KEY_INSERT;
  io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
  io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
  io.KeyMap[ImGuiKey_Space] = GLFW_KEY_SPACE;
  io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
  io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
  io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
  io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
  io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
  io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
  io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
  io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;

  //io.SetClipboardTextFn
  //io.GetClipboardTextFn
  //io.ClipboardUserData

  // We use this map to easily have GLFW update the cursor depending on what
  // imgui decides it to be.
  mouse_cursors_ = new GLFWcursor*[ImGuiMouseCursor_COUNT];
  mouse_cursors_[ImGuiMouseCursor_Arrow] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
  mouse_cursors_[ImGuiMouseCursor_TextInput] = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
  mouse_cursors_[ImGuiMouseCursor_ResizeAll] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
  mouse_cursors_[ImGuiMouseCursor_ResizeNS] = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
  mouse_cursors_[ImGuiMouseCursor_ResizeEW] = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
  mouse_cursors_[ImGuiMouseCursor_ResizeNESW] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
  mouse_cursors_[ImGuiMouseCursor_ResizeNWSE] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);

  camera_.reset(new Camera);
  camera_->LookAt(DirectX::XMFLOAT3 { 0.0f, 0.0f, 0.0f }, DirectX::XMFLOAT3 { 0.0f, 0.0f, 1.0f }, DirectX::XMFLOAT3 { 0.0f, 1.0f, 0.0f });
  camera_->UpdateViewMatrix();
  // Well, no matter what I do I can't get a proper (that is, around 90 degrees)
  // FoV to work without ridiculous amounts of perspective distortion. This
  // will have to suffice.
  camera_->SetLens(DirectX::XMConvertToRadians(40.0f), window_width_ / static_cast<float>(window_height_), 1000.0f, 0.1f);

  renderer_.reset(new Renderer(glfwGetWin32Window(window_), window_width_, window_height_));
  scene_.reset(new Scene { *renderer_ });

  float time = glfwGetTime();

  while (!glfwWindowShouldClose(window_)) {
    float t = glfwGetTime();
    float delta_time = t - time;
    time = t;

    frame(delta_time, time);

    // TODO: In the callback I probably have to use io.WantCaptureMouse and io.WantCaptureKeyboard
    // to see if imgui has taken the input.
    glfwPollEvents();
  }

  ImGui::DestroyContext();

  for (ImGuiMouseCursor c = 0; c < ImGuiMouseCursor_COUNT; ++c) {
    glfwDestroyCursor(mouse_cursors_[c]);
  }
  delete[] mouse_cursors_;
  mouse_cursors_ = nullptr;

  glfwDestroyWindow(window_);
  window_ = nullptr;
  glfwTerminate();

  renderer_.reset(nullptr);

  std::cin.get();
}

App::App() = default;
App::~App() = default;

void App::frame(float delta_time, float total_time) {
  int w, h;
  glfwGetWindowSize(window_, &w, &h);

  // Update ImGui
  ImGuiIO& io = ImGui::GetIO();
  io.DisplaySize = ImVec2(static_cast<float>(w), static_cast<float>(h));
  io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
  io.DeltaTime = delta_time;

  if (glfwGetWindowAttrib(window_, GLFW_FOCUSED)) {
    double mouse_x, mouse_y;
    glfwGetCursorPos(window_, &mouse_x, &mouse_y);
    io.MousePos = ImVec2(static_cast<float>(mouse_x), static_cast<float>(mouse_y));
  }
  else {
#pragma push_macro("min")
#undef min
#pragma push_macro("max")
#undef max
    io.MousePos = ImVec2(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
#pragma pop_macro("max")
#pragma pop_macro("min")
  }

  for (int i = 0; i < 3; ++i) {
    io.MouseDown[i] = /* g_MouseJustPressed[i] || */glfwGetMouseButton(window_, i) != 0;
    // g_MouseJustPressed[i] = false;
  }

  // Update OS/hardware mouse cursor if imgui isn't drawing a software cursor
  if ((io.ConfigFlags & ImGuiConfigFlags_NoSetMouseCursor) == 0) {
    ImGuiMouseCursor cursor = ImGui::GetMouseCursor();
    if (io.MouseDrawCursor || cursor == ImGuiMouseCursor_None) {
      glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    }
    else {
      glfwSetCursor(window_, mouse_cursors_[cursor] ? mouse_cursors_[cursor] : mouse_cursors_[ImGuiMouseCursor_Arrow]);
      glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
  }

  // Start ImGui frame. This updates io.WantCaptureMouse and io.WantCaptureKeyboard.
  ImGui::NewFrame();

  scene_->update(delta_time, *renderer_);

  ImGui::Render();

  renderer_->use_matrices(camera_->View(), camera_->Proj());
  renderer_->render();
}

void App::key_callback(int key, int scancode, int action, int mods) {
  if (key == GLFW_KEY_ESCAPE) {
    glfwSetWindowShouldClose(window_, GLFW_TRUE);
  }
}
