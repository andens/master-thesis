#pragma once

#include <memory>
#include "vulkan_include.inl"

namespace vk {

class Instance;

class PresentationSurface {
public:
#if defined(_WIN32)
  static std::unique_ptr<PresentationSurface> for_win32(HWND hwnd,
      std::weak_ptr<Instance> instance);
#endif

  VkSurfaceKHR surface() const { return surface_; }

#if defined(_WIN32)
  HWND win32_hwnd() const { return hwnd_; }
#endif

  ~PresentationSurface();

private:
  PresentationSurface() {}
  PresentationSurface(PresentationSurface& other) = delete;
  void operator=(PresentationSurface& rhs) = delete;

private:
  VkSurfaceKHR surface_ = VK_NULL_HANDLE;
  std::weak_ptr<Instance> instance_;

#if defined(_WIN32)
  HWND hwnd_ = NULL;
#endif
};

}
