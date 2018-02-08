#include "presentation_surface.h"

#include <stdexcept>
#include "instance.h"

using namespace std;

namespace vk {

#if defined(_WIN32)
unique_ptr<PresentationSurface> PresentationSurface::for_win32(HWND hwnd,
      weak_ptr<vk::Instance> instance) {
  VkWin32SurfaceCreateInfoKHR surface_info = {};
  surface_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
  surface_info.hinstance = GetModuleHandle(nullptr);
  surface_info.hwnd = hwnd;

  VkSurfaceKHR surface = VK_NULL_HANDLE;
  VkResult result = instance.lock()->vkCreateWin32SurfaceKHR(
      &surface_info, nullptr, &surface);

  if (result != VK_SUCCESS) {
    throw runtime_error("Failed to create presentation surface.");
  }

  auto ptr = unique_ptr<PresentationSurface>(new PresentationSurface());
  ptr->surface_ = surface;
  ptr->instance_ = instance;
  ptr->hwnd_ = hwnd;
  return ptr;
}
#endif

PresentationSurface::~PresentationSurface() {
  if (surface_) {
    instance_.lock()->vkDestroySurfaceKHR(surface_, nullptr);
    surface_ = VK_NULL_HANDLE;
  }
}

}
