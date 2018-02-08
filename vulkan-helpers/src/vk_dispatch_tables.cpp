#include "vk_dispatch_tables.h"

#include <stdexcept>

namespace vkgen {

/*
 * ------------------------------------------------------
 * GlobalFunctions
 * ------------------------------------------------------
*/

GlobalFunctions::GlobalFunctions(std::string const& vulkan_library) {
#if defined(_WIN32)
  library_ = LoadLibraryA(vulkan_library.c_str());
#elif defined(__linux__)
  library_ = dlopen(vulkan_library.c_str(), RTLD_NOW);
#else
#error "Unsupported OS"
#endif

  if (!library_) {
    throw std::runtime_error("Could not load Vulkan loader.");
  }

#if defined(_WIN32)
  vkGetInstanceProcAddr_ = reinterpret_cast<PFN_vkGetInstanceProcAddr>(
      GetProcAddress(library_, "vkGetInstanceProcAddr"));
#elif defined(__linux__)
  vkGetInstanceProcAddr_ = reinterpret_cast<PFN_vkGetInstanceProcAddr>(
      dlsym(library_, "vkGetInstanceProcAddr"));
#else
#error "Unsupported OS"
#endif

  if (!vkGetInstanceProcAddr_) {
    throw VulkanProcNotFound("vkGetInstanceProcAddr");
  }

  vkCreateInstance_ = reinterpret_cast<PFN_vkCreateInstance>(this->vkGetInstanceProcAddr(nullptr, "vkCreateInstance"));
  if (!vkCreateInstance_) {
    throw VulkanProcNotFound("vkCreateInstance");
  }
  vkEnumerateInstanceExtensionProperties_ = reinterpret_cast<PFN_vkEnumerateInstanceExtensionProperties>(this->vkGetInstanceProcAddr(nullptr, "vkEnumerateInstanceExtensionProperties"));
  if (!vkEnumerateInstanceExtensionProperties_) {
    throw VulkanProcNotFound("vkEnumerateInstanceExtensionProperties");
  }
  vkEnumerateInstanceLayerProperties_ = reinterpret_cast<PFN_vkEnumerateInstanceLayerProperties>(this->vkGetInstanceProcAddr(nullptr, "vkEnumerateInstanceLayerProperties"));
  if (!vkEnumerateInstanceLayerProperties_) {
    throw VulkanProcNotFound("vkEnumerateInstanceLayerProperties");
  }
}

GlobalFunctions::~GlobalFunctions() {
#if defined(_WIN32)
  FreeLibrary(library_);
#elif defined(__linux__)
  dlclose(library_);
#else
#error "Unsupported OS"
#endif
}

PFN_vkVoidFunction GlobalFunctions::vkGetInstanceProcAddr(VkInstance instance, const char* pName) const {
  return this->vkGetInstanceProcAddr_(instance, pName);
}

VkResult GlobalFunctions::vkCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance) const {
  return this->vkCreateInstance_(pCreateInfo, pAllocator, pInstance);
}

VkResult GlobalFunctions::vkEnumerateInstanceExtensionProperties(const char* pLayerName, uint32_t* pPropertyCount, VkExtensionProperties* pProperties) const {
  return this->vkEnumerateInstanceExtensionProperties_(pLayerName, pPropertyCount, pProperties);
}

VkResult GlobalFunctions::vkEnumerateInstanceLayerProperties(uint32_t* pPropertyCount, VkLayerProperties* pProperties) const {
  return this->vkEnumerateInstanceLayerProperties_(pPropertyCount, pProperties);
}

/*
 * ------------------------------------------------------
 * InstanceFunctions
 * ------------------------------------------------------
*/

PFN_vkVoidFunction InstanceFunctions::vkGetInstanceProcAddr(const char* pName) const {
  return this->vkGetInstanceProcAddr_(instance_, pName);
}

void InstanceFunctions::vkDestroyInstance(const VkAllocationCallbacks* pAllocator) const {
  return this->vkDestroyInstance_(instance_, pAllocator);
}

VkResult InstanceFunctions::vkEnumeratePhysicalDevices(uint32_t* pPhysicalDeviceCount, VkPhysicalDevice* pPhysicalDevices) const {
  return this->vkEnumeratePhysicalDevices_(instance_, pPhysicalDeviceCount, pPhysicalDevices);
}

void InstanceFunctions::vkDestroySurfaceKHR(VkSurfaceKHR surface, const VkAllocationCallbacks* pAllocator) const {
  return this->vkDestroySurfaceKHR_(instance_, surface, pAllocator);
}

VkResult InstanceFunctions::vkCreateDisplayPlaneSurfaceKHR(const VkDisplaySurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
  return this->vkCreateDisplayPlaneSurfaceKHR_(instance_, pCreateInfo, pAllocator, pSurface);
}

#if defined(VK_USE_PLATFORM_XLIB_KHR)
VkResult InstanceFunctions::vkCreateXlibSurfaceKHR(const VkXlibSurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
  return this->vkCreateXlibSurfaceKHR_(instance_, pCreateInfo, pAllocator, pSurface);
}
#endif

#if defined(VK_USE_PLATFORM_XCB_KHR)
VkResult InstanceFunctions::vkCreateXcbSurfaceKHR(const VkXcbSurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
  return this->vkCreateXcbSurfaceKHR_(instance_, pCreateInfo, pAllocator, pSurface);
}
#endif

#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
VkResult InstanceFunctions::vkCreateWaylandSurfaceKHR(const VkWaylandSurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
  return this->vkCreateWaylandSurfaceKHR_(instance_, pCreateInfo, pAllocator, pSurface);
}
#endif

#if defined(VK_USE_PLATFORM_MIR_KHR)
VkResult InstanceFunctions::vkCreateMirSurfaceKHR(const VkMirSurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
  return this->vkCreateMirSurfaceKHR_(instance_, pCreateInfo, pAllocator, pSurface);
}
#endif

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
VkResult InstanceFunctions::vkCreateAndroidSurfaceKHR(const VkAndroidSurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
  return this->vkCreateAndroidSurfaceKHR_(instance_, pCreateInfo, pAllocator, pSurface);
}
#endif

#if defined(VK_USE_PLATFORM_WIN32_KHR)
VkResult InstanceFunctions::vkCreateWin32SurfaceKHR(const VkWin32SurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
  return this->vkCreateWin32SurfaceKHR_(instance_, pCreateInfo, pAllocator, pSurface);
}
#endif

VkResult InstanceFunctions::vkCreateDebugReportCallbackEXT(const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback) const {
  return this->vkCreateDebugReportCallbackEXT_(instance_, pCreateInfo, pAllocator, pCallback);
}

void InstanceFunctions::vkDestroyDebugReportCallbackEXT(VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator) const {
  return this->vkDestroyDebugReportCallbackEXT_(instance_, callback, pAllocator);
}

void InstanceFunctions::vkDebugReportMessageEXT(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage) const {
  return this->vkDebugReportMessageEXT_(instance_, flags, objectType, object, location, messageCode, pLayerPrefix, pMessage);
}

#if defined(VK_USE_PLATFORM_VI_NN)
VkResult InstanceFunctions::vkCreateViSurfaceNN(const VkViSurfaceCreateInfoNN* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
  return this->vkCreateViSurfaceNN_(instance_, pCreateInfo, pAllocator, pSurface);
}
#endif

VkResult InstanceFunctions::vkEnumeratePhysicalDeviceGroupsKHX(uint32_t* pPhysicalDeviceGroupCount, VkPhysicalDeviceGroupPropertiesKHX* pPhysicalDeviceGroupProperties) const {
  return this->vkEnumeratePhysicalDeviceGroupsKHX_(instance_, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
}

#if defined(VK_USE_PLATFORM_IOS_MVK)
VkResult InstanceFunctions::vkCreateIOSSurfaceMVK(const VkIOSSurfaceCreateInfoMVK* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
  return this->vkCreateIOSSurfaceMVK_(instance_, pCreateInfo, pAllocator, pSurface);
}
#endif

#if defined(VK_USE_PLATFORM_MACOS_MVK)
VkResult InstanceFunctions::vkCreateMacOSSurfaceMVK(const VkMacOSSurfaceCreateInfoMVK* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
  return this->vkCreateMacOSSurfaceMVK_(instance_, pCreateInfo, pAllocator, pSurface);
}
#endif

InstanceFunctions::InstanceFunctions(VkInstance instance, GlobalFunctions* globals) {
  instance_ = instance;
  vkGetInstanceProcAddr_ = reinterpret_cast<PFN_vkGetInstanceProcAddr>(globals->vkGetInstanceProcAddr(instance, "vkGetInstanceProcAddr"));
  if (!vkGetInstanceProcAddr_) {
    throw VulkanProcNotFound("vkGetInstanceProcAddr");
  }
  vkDestroyInstance_ = reinterpret_cast<PFN_vkDestroyInstance>(this->vkGetInstanceProcAddr("vkDestroyInstance"));
  if (!vkDestroyInstance_) {
    throw VulkanProcNotFound("vkDestroyInstance");
  }
  vkEnumeratePhysicalDevices_ = reinterpret_cast<PFN_vkEnumeratePhysicalDevices>(this->vkGetInstanceProcAddr("vkEnumeratePhysicalDevices"));
  if (!vkEnumeratePhysicalDevices_) {
    throw VulkanProcNotFound("vkEnumeratePhysicalDevices");
  }
  vkDestroySurfaceKHR_ = reinterpret_cast<PFN_vkDestroySurfaceKHR>(this->vkGetInstanceProcAddr("vkDestroySurfaceKHR"));
  vkCreateDisplayPlaneSurfaceKHR_ = reinterpret_cast<PFN_vkCreateDisplayPlaneSurfaceKHR>(this->vkGetInstanceProcAddr("vkCreateDisplayPlaneSurfaceKHR"));
#if defined(VK_USE_PLATFORM_XLIB_KHR)
  vkCreateXlibSurfaceKHR_ = reinterpret_cast<PFN_vkCreateXlibSurfaceKHR>(this->vkGetInstanceProcAddr("vkCreateXlibSurfaceKHR"));
#endif
#if defined(VK_USE_PLATFORM_XCB_KHR)
  vkCreateXcbSurfaceKHR_ = reinterpret_cast<PFN_vkCreateXcbSurfaceKHR>(this->vkGetInstanceProcAddr("vkCreateXcbSurfaceKHR"));
#endif
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
  vkCreateWaylandSurfaceKHR_ = reinterpret_cast<PFN_vkCreateWaylandSurfaceKHR>(this->vkGetInstanceProcAddr("vkCreateWaylandSurfaceKHR"));
#endif
#if defined(VK_USE_PLATFORM_MIR_KHR)
  vkCreateMirSurfaceKHR_ = reinterpret_cast<PFN_vkCreateMirSurfaceKHR>(this->vkGetInstanceProcAddr("vkCreateMirSurfaceKHR"));
#endif
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
  vkCreateAndroidSurfaceKHR_ = reinterpret_cast<PFN_vkCreateAndroidSurfaceKHR>(this->vkGetInstanceProcAddr("vkCreateAndroidSurfaceKHR"));
#endif
#if defined(VK_USE_PLATFORM_WIN32_KHR)
  vkCreateWin32SurfaceKHR_ = reinterpret_cast<PFN_vkCreateWin32SurfaceKHR>(this->vkGetInstanceProcAddr("vkCreateWin32SurfaceKHR"));
#endif
  vkCreateDebugReportCallbackEXT_ = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(this->vkGetInstanceProcAddr("vkCreateDebugReportCallbackEXT"));
  vkDestroyDebugReportCallbackEXT_ = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(this->vkGetInstanceProcAddr("vkDestroyDebugReportCallbackEXT"));
  vkDebugReportMessageEXT_ = reinterpret_cast<PFN_vkDebugReportMessageEXT>(this->vkGetInstanceProcAddr("vkDebugReportMessageEXT"));
#if defined(VK_USE_PLATFORM_VI_NN)
  vkCreateViSurfaceNN_ = reinterpret_cast<PFN_vkCreateViSurfaceNN>(this->vkGetInstanceProcAddr("vkCreateViSurfaceNN"));
#endif
  vkEnumeratePhysicalDeviceGroupsKHX_ = reinterpret_cast<PFN_vkEnumeratePhysicalDeviceGroupsKHX>(this->vkGetInstanceProcAddr("vkEnumeratePhysicalDeviceGroupsKHX"));
#if defined(VK_USE_PLATFORM_IOS_MVK)
  vkCreateIOSSurfaceMVK_ = reinterpret_cast<PFN_vkCreateIOSSurfaceMVK>(this->vkGetInstanceProcAddr("vkCreateIOSSurfaceMVK"));
#endif
#if defined(VK_USE_PLATFORM_MACOS_MVK)
  vkCreateMacOSSurfaceMVK_ = reinterpret_cast<PFN_vkCreateMacOSSurfaceMVK>(this->vkGetInstanceProcAddr("vkCreateMacOSSurfaceMVK"));
#endif
}

/*
 * ------------------------------------------------------
 * PhysicalDeviceFunctions
 * ------------------------------------------------------
*/

void PhysicalDeviceFunctions::vkGetPhysicalDeviceFeatures(VkPhysicalDeviceFeatures* pFeatures) const {
  return this->vkGetPhysicalDeviceFeatures_(physical_device_, pFeatures);
}

void PhysicalDeviceFunctions::vkGetPhysicalDeviceFormatProperties(VkFormat format, VkFormatProperties* pFormatProperties) const {
  return this->vkGetPhysicalDeviceFormatProperties_(physical_device_, format, pFormatProperties);
}

VkResult PhysicalDeviceFunctions::vkGetPhysicalDeviceImageFormatProperties(VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags, VkImageFormatProperties* pImageFormatProperties) const {
  return this->vkGetPhysicalDeviceImageFormatProperties_(physical_device_, format, type, tiling, usage, flags, pImageFormatProperties);
}

void PhysicalDeviceFunctions::vkGetPhysicalDeviceProperties(VkPhysicalDeviceProperties* pProperties) const {
  return this->vkGetPhysicalDeviceProperties_(physical_device_, pProperties);
}

void PhysicalDeviceFunctions::vkGetPhysicalDeviceQueueFamilyProperties(uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties* pQueueFamilyProperties) const {
  return this->vkGetPhysicalDeviceQueueFamilyProperties_(physical_device_, pQueueFamilyPropertyCount, pQueueFamilyProperties);
}

void PhysicalDeviceFunctions::vkGetPhysicalDeviceMemoryProperties(VkPhysicalDeviceMemoryProperties* pMemoryProperties) const {
  return this->vkGetPhysicalDeviceMemoryProperties_(physical_device_, pMemoryProperties);
}

VkResult PhysicalDeviceFunctions::vkCreateDevice(const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice) const {
  return this->vkCreateDevice_(physical_device_, pCreateInfo, pAllocator, pDevice);
}

VkResult PhysicalDeviceFunctions::vkEnumerateDeviceExtensionProperties(const char* pLayerName, uint32_t* pPropertyCount, VkExtensionProperties* pProperties) const {
  return this->vkEnumerateDeviceExtensionProperties_(physical_device_, pLayerName, pPropertyCount, pProperties);
}

VkResult PhysicalDeviceFunctions::vkEnumerateDeviceLayerProperties(uint32_t* pPropertyCount, VkLayerProperties* pProperties) const {
  return this->vkEnumerateDeviceLayerProperties_(physical_device_, pPropertyCount, pProperties);
}

void PhysicalDeviceFunctions::vkGetPhysicalDeviceSparseImageFormatProperties(VkFormat format, VkImageType type, VkSampleCountFlagBits samples, VkImageUsageFlags usage, VkImageTiling tiling, uint32_t* pPropertyCount, VkSparseImageFormatProperties* pProperties) const {
  return this->vkGetPhysicalDeviceSparseImageFormatProperties_(physical_device_, format, type, samples, usage, tiling, pPropertyCount, pProperties);
}

VkResult PhysicalDeviceFunctions::vkGetPhysicalDeviceSurfaceSupportKHR(uint32_t queueFamilyIndex, VkSurfaceKHR surface, VkBool32* pSupported) const {
  return this->vkGetPhysicalDeviceSurfaceSupportKHR_(physical_device_, queueFamilyIndex, surface, pSupported);
}

VkResult PhysicalDeviceFunctions::vkGetPhysicalDeviceSurfaceCapabilitiesKHR(VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR* pSurfaceCapabilities) const {
  return this->vkGetPhysicalDeviceSurfaceCapabilitiesKHR_(physical_device_, surface, pSurfaceCapabilities);
}

VkResult PhysicalDeviceFunctions::vkGetPhysicalDeviceSurfaceFormatsKHR(VkSurfaceKHR surface, uint32_t* pSurfaceFormatCount, VkSurfaceFormatKHR* pSurfaceFormats) const {
  return this->vkGetPhysicalDeviceSurfaceFormatsKHR_(physical_device_, surface, pSurfaceFormatCount, pSurfaceFormats);
}

VkResult PhysicalDeviceFunctions::vkGetPhysicalDeviceSurfacePresentModesKHR(VkSurfaceKHR surface, uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes) const {
  return this->vkGetPhysicalDeviceSurfacePresentModesKHR_(physical_device_, surface, pPresentModeCount, pPresentModes);
}

VkResult PhysicalDeviceFunctions::vkGetPhysicalDeviceDisplayPropertiesKHR(uint32_t* pPropertyCount, VkDisplayPropertiesKHR* pProperties) const {
  return this->vkGetPhysicalDeviceDisplayPropertiesKHR_(physical_device_, pPropertyCount, pProperties);
}

VkResult PhysicalDeviceFunctions::vkGetPhysicalDeviceDisplayPlanePropertiesKHR(uint32_t* pPropertyCount, VkDisplayPlanePropertiesKHR* pProperties) const {
  return this->vkGetPhysicalDeviceDisplayPlanePropertiesKHR_(physical_device_, pPropertyCount, pProperties);
}

VkResult PhysicalDeviceFunctions::vkGetDisplayPlaneSupportedDisplaysKHR(uint32_t planeIndex, uint32_t* pDisplayCount, VkDisplayKHR* pDisplays) const {
  return this->vkGetDisplayPlaneSupportedDisplaysKHR_(physical_device_, planeIndex, pDisplayCount, pDisplays);
}

VkResult PhysicalDeviceFunctions::vkGetDisplayModePropertiesKHR(VkDisplayKHR display, uint32_t* pPropertyCount, VkDisplayModePropertiesKHR* pProperties) const {
  return this->vkGetDisplayModePropertiesKHR_(physical_device_, display, pPropertyCount, pProperties);
}

VkResult PhysicalDeviceFunctions::vkCreateDisplayModeKHR(VkDisplayKHR display, const VkDisplayModeCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDisplayModeKHR* pMode) const {
  return this->vkCreateDisplayModeKHR_(physical_device_, display, pCreateInfo, pAllocator, pMode);
}

VkResult PhysicalDeviceFunctions::vkGetDisplayPlaneCapabilitiesKHR(VkDisplayModeKHR mode, uint32_t planeIndex, VkDisplayPlaneCapabilitiesKHR* pCapabilities) const {
  return this->vkGetDisplayPlaneCapabilitiesKHR_(physical_device_, mode, planeIndex, pCapabilities);
}

#if defined(VK_USE_PLATFORM_XLIB_KHR)
VkBool32 PhysicalDeviceFunctions::vkGetPhysicalDeviceXlibPresentationSupportKHR(uint32_t queueFamilyIndex, Display* dpy, VisualID visualID) const {
  return this->vkGetPhysicalDeviceXlibPresentationSupportKHR_(physical_device_, queueFamilyIndex, dpy, visualID);
}
#endif

#if defined(VK_USE_PLATFORM_XCB_KHR)
VkBool32 PhysicalDeviceFunctions::vkGetPhysicalDeviceXcbPresentationSupportKHR(uint32_t queueFamilyIndex, xcb_connection_t* connection, xcb_visualid_t visual_id) const {
  return this->vkGetPhysicalDeviceXcbPresentationSupportKHR_(physical_device_, queueFamilyIndex, connection, visual_id);
}
#endif

#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
VkBool32 PhysicalDeviceFunctions::vkGetPhysicalDeviceWaylandPresentationSupportKHR(uint32_t queueFamilyIndex, wl_display* display) const {
  return this->vkGetPhysicalDeviceWaylandPresentationSupportKHR_(physical_device_, queueFamilyIndex, display);
}
#endif

#if defined(VK_USE_PLATFORM_MIR_KHR)
VkBool32 PhysicalDeviceFunctions::vkGetPhysicalDeviceMirPresentationSupportKHR(uint32_t queueFamilyIndex, MirConnection* connection) const {
  return this->vkGetPhysicalDeviceMirPresentationSupportKHR_(physical_device_, queueFamilyIndex, connection);
}
#endif

#if defined(VK_USE_PLATFORM_WIN32_KHR)
VkBool32 PhysicalDeviceFunctions::vkGetPhysicalDeviceWin32PresentationSupportKHR(uint32_t queueFamilyIndex) const {
  return this->vkGetPhysicalDeviceWin32PresentationSupportKHR_(physical_device_, queueFamilyIndex);
}
#endif

VkResult PhysicalDeviceFunctions::vkGetPhysicalDeviceExternalImageFormatPropertiesNV(VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags, VkExternalMemoryHandleTypeFlagsNV externalHandleType, VkExternalImageFormatPropertiesNV* pExternalImageFormatProperties) const {
  return this->vkGetPhysicalDeviceExternalImageFormatPropertiesNV_(physical_device_, format, type, tiling, usage, flags, externalHandleType, pExternalImageFormatProperties);
}

void PhysicalDeviceFunctions::vkGetPhysicalDeviceFeatures2KHR(VkPhysicalDeviceFeatures2KHR* pFeatures) const {
  return this->vkGetPhysicalDeviceFeatures2KHR_(physical_device_, pFeatures);
}

void PhysicalDeviceFunctions::vkGetPhysicalDeviceProperties2KHR(VkPhysicalDeviceProperties2KHR* pProperties) const {
  return this->vkGetPhysicalDeviceProperties2KHR_(physical_device_, pProperties);
}

void PhysicalDeviceFunctions::vkGetPhysicalDeviceFormatProperties2KHR(VkFormat format, VkFormatProperties2KHR* pFormatProperties) const {
  return this->vkGetPhysicalDeviceFormatProperties2KHR_(physical_device_, format, pFormatProperties);
}

VkResult PhysicalDeviceFunctions::vkGetPhysicalDeviceImageFormatProperties2KHR(const VkPhysicalDeviceImageFormatInfo2KHR* pImageFormatInfo, VkImageFormatProperties2KHR* pImageFormatProperties) const {
  return this->vkGetPhysicalDeviceImageFormatProperties2KHR_(physical_device_, pImageFormatInfo, pImageFormatProperties);
}

void PhysicalDeviceFunctions::vkGetPhysicalDeviceQueueFamilyProperties2KHR(uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties2KHR* pQueueFamilyProperties) const {
  return this->vkGetPhysicalDeviceQueueFamilyProperties2KHR_(physical_device_, pQueueFamilyPropertyCount, pQueueFamilyProperties);
}

void PhysicalDeviceFunctions::vkGetPhysicalDeviceMemoryProperties2KHR(VkPhysicalDeviceMemoryProperties2KHR* pMemoryProperties) const {
  return this->vkGetPhysicalDeviceMemoryProperties2KHR_(physical_device_, pMemoryProperties);
}

void PhysicalDeviceFunctions::vkGetPhysicalDeviceSparseImageFormatProperties2KHR(const VkPhysicalDeviceSparseImageFormatInfo2KHR* pFormatInfo, uint32_t* pPropertyCount, VkSparseImageFormatProperties2KHR* pProperties) const {
  return this->vkGetPhysicalDeviceSparseImageFormatProperties2KHR_(physical_device_, pFormatInfo, pPropertyCount, pProperties);
}

void PhysicalDeviceFunctions::vkGetPhysicalDeviceExternalBufferPropertiesKHR(const VkPhysicalDeviceExternalBufferInfoKHR* pExternalBufferInfo, VkExternalBufferPropertiesKHR* pExternalBufferProperties) const {
  return this->vkGetPhysicalDeviceExternalBufferPropertiesKHR_(physical_device_, pExternalBufferInfo, pExternalBufferProperties);
}

void PhysicalDeviceFunctions::vkGetPhysicalDeviceExternalSemaphorePropertiesKHR(const VkPhysicalDeviceExternalSemaphoreInfoKHR* pExternalSemaphoreInfo, VkExternalSemaphorePropertiesKHR* pExternalSemaphoreProperties) const {
  return this->vkGetPhysicalDeviceExternalSemaphorePropertiesKHR_(physical_device_, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
}

void PhysicalDeviceFunctions::vkGetPhysicalDeviceGeneratedCommandsPropertiesNVX(VkDeviceGeneratedCommandsFeaturesNVX* pFeatures, VkDeviceGeneratedCommandsLimitsNVX* pLimits) const {
  return this->vkGetPhysicalDeviceGeneratedCommandsPropertiesNVX_(physical_device_, pFeatures, pLimits);
}

VkResult PhysicalDeviceFunctions::vkReleaseDisplayEXT(VkDisplayKHR display) const {
  return this->vkReleaseDisplayEXT_(physical_device_, display);
}

#if defined(VK_USE_PLATFORM_XLIB_XRANDR_EXT)
VkResult PhysicalDeviceFunctions::vkAcquireXlibDisplayEXT(Display* dpy, VkDisplayKHR display) const {
  return this->vkAcquireXlibDisplayEXT_(physical_device_, dpy, display);
}
#endif

#if defined(VK_USE_PLATFORM_XLIB_XRANDR_EXT)
VkResult PhysicalDeviceFunctions::vkGetRandROutputDisplayEXT(Display* dpy, RROutput rrOutput, VkDisplayKHR* pDisplay) const {
  return this->vkGetRandROutputDisplayEXT_(physical_device_, dpy, rrOutput, pDisplay);
}
#endif

VkResult PhysicalDeviceFunctions::vkGetPhysicalDeviceSurfaceCapabilities2EXT(VkSurfaceKHR surface, VkSurfaceCapabilities2EXT* pSurfaceCapabilities) const {
  return this->vkGetPhysicalDeviceSurfaceCapabilities2EXT_(physical_device_, surface, pSurfaceCapabilities);
}

void PhysicalDeviceFunctions::vkGetPhysicalDeviceExternalFencePropertiesKHR(const VkPhysicalDeviceExternalFenceInfoKHR* pExternalFenceInfo, VkExternalFencePropertiesKHR* pExternalFenceProperties) const {
  return this->vkGetPhysicalDeviceExternalFencePropertiesKHR_(physical_device_, pExternalFenceInfo, pExternalFenceProperties);
}

VkResult PhysicalDeviceFunctions::vkGetPhysicalDeviceSurfaceCapabilities2KHR(const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo, VkSurfaceCapabilities2KHR* pSurfaceCapabilities) const {
  return this->vkGetPhysicalDeviceSurfaceCapabilities2KHR_(physical_device_, pSurfaceInfo, pSurfaceCapabilities);
}

VkResult PhysicalDeviceFunctions::vkGetPhysicalDeviceSurfaceFormats2KHR(const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo, uint32_t* pSurfaceFormatCount, VkSurfaceFormat2KHR* pSurfaceFormats) const {
  return this->vkGetPhysicalDeviceSurfaceFormats2KHR_(physical_device_, pSurfaceInfo, pSurfaceFormatCount, pSurfaceFormats);
}

void PhysicalDeviceFunctions::vkGetPhysicalDeviceMultisamplePropertiesEXT(VkSampleCountFlagBits samples, VkMultisamplePropertiesEXT* pMultisampleProperties) const {
  return this->vkGetPhysicalDeviceMultisamplePropertiesEXT_(physical_device_, samples, pMultisampleProperties);
}

PhysicalDeviceFunctions::PhysicalDeviceFunctions(VkPhysicalDevice physical_device, InstanceFunctions* instance) {
  physical_device_ = physical_device;
  vkGetPhysicalDeviceFeatures_ = reinterpret_cast<PFN_vkGetPhysicalDeviceFeatures>(instance->vkGetInstanceProcAddr("vkGetPhysicalDeviceFeatures"));
  if (!vkGetPhysicalDeviceFeatures_) {
    throw VulkanProcNotFound("vkGetPhysicalDeviceFeatures");
  }
  vkGetPhysicalDeviceFormatProperties_ = reinterpret_cast<PFN_vkGetPhysicalDeviceFormatProperties>(instance->vkGetInstanceProcAddr("vkGetPhysicalDeviceFormatProperties"));
  if (!vkGetPhysicalDeviceFormatProperties_) {
    throw VulkanProcNotFound("vkGetPhysicalDeviceFormatProperties");
  }
  vkGetPhysicalDeviceImageFormatProperties_ = reinterpret_cast<PFN_vkGetPhysicalDeviceImageFormatProperties>(instance->vkGetInstanceProcAddr("vkGetPhysicalDeviceImageFormatProperties"));
  if (!vkGetPhysicalDeviceImageFormatProperties_) {
    throw VulkanProcNotFound("vkGetPhysicalDeviceImageFormatProperties");
  }
  vkGetPhysicalDeviceProperties_ = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties>(instance->vkGetInstanceProcAddr("vkGetPhysicalDeviceProperties"));
  if (!vkGetPhysicalDeviceProperties_) {
    throw VulkanProcNotFound("vkGetPhysicalDeviceProperties");
  }
  vkGetPhysicalDeviceQueueFamilyProperties_ = reinterpret_cast<PFN_vkGetPhysicalDeviceQueueFamilyProperties>(instance->vkGetInstanceProcAddr("vkGetPhysicalDeviceQueueFamilyProperties"));
  if (!vkGetPhysicalDeviceQueueFamilyProperties_) {
    throw VulkanProcNotFound("vkGetPhysicalDeviceQueueFamilyProperties");
  }
  vkGetPhysicalDeviceMemoryProperties_ = reinterpret_cast<PFN_vkGetPhysicalDeviceMemoryProperties>(instance->vkGetInstanceProcAddr("vkGetPhysicalDeviceMemoryProperties"));
  if (!vkGetPhysicalDeviceMemoryProperties_) {
    throw VulkanProcNotFound("vkGetPhysicalDeviceMemoryProperties");
  }
  vkCreateDevice_ = reinterpret_cast<PFN_vkCreateDevice>(instance->vkGetInstanceProcAddr("vkCreateDevice"));
  if (!vkCreateDevice_) {
    throw VulkanProcNotFound("vkCreateDevice");
  }
  vkEnumerateDeviceExtensionProperties_ = reinterpret_cast<PFN_vkEnumerateDeviceExtensionProperties>(instance->vkGetInstanceProcAddr("vkEnumerateDeviceExtensionProperties"));
  if (!vkEnumerateDeviceExtensionProperties_) {
    throw VulkanProcNotFound("vkEnumerateDeviceExtensionProperties");
  }
  vkEnumerateDeviceLayerProperties_ = reinterpret_cast<PFN_vkEnumerateDeviceLayerProperties>(instance->vkGetInstanceProcAddr("vkEnumerateDeviceLayerProperties"));
  if (!vkEnumerateDeviceLayerProperties_) {
    throw VulkanProcNotFound("vkEnumerateDeviceLayerProperties");
  }
  vkGetPhysicalDeviceSparseImageFormatProperties_ = reinterpret_cast<PFN_vkGetPhysicalDeviceSparseImageFormatProperties>(instance->vkGetInstanceProcAddr("vkGetPhysicalDeviceSparseImageFormatProperties"));
  if (!vkGetPhysicalDeviceSparseImageFormatProperties_) {
    throw VulkanProcNotFound("vkGetPhysicalDeviceSparseImageFormatProperties");
  }
  vkGetPhysicalDeviceSurfaceSupportKHR_ = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceSupportKHR>(instance->vkGetInstanceProcAddr("vkGetPhysicalDeviceSurfaceSupportKHR"));
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR_ = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR>(instance->vkGetInstanceProcAddr("vkGetPhysicalDeviceSurfaceCapabilitiesKHR"));
  vkGetPhysicalDeviceSurfaceFormatsKHR_ = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceFormatsKHR>(instance->vkGetInstanceProcAddr("vkGetPhysicalDeviceSurfaceFormatsKHR"));
  vkGetPhysicalDeviceSurfacePresentModesKHR_ = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfacePresentModesKHR>(instance->vkGetInstanceProcAddr("vkGetPhysicalDeviceSurfacePresentModesKHR"));
  vkGetPhysicalDeviceDisplayPropertiesKHR_ = reinterpret_cast<PFN_vkGetPhysicalDeviceDisplayPropertiesKHR>(instance->vkGetInstanceProcAddr("vkGetPhysicalDeviceDisplayPropertiesKHR"));
  vkGetPhysicalDeviceDisplayPlanePropertiesKHR_ = reinterpret_cast<PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR>(instance->vkGetInstanceProcAddr("vkGetPhysicalDeviceDisplayPlanePropertiesKHR"));
  vkGetDisplayPlaneSupportedDisplaysKHR_ = reinterpret_cast<PFN_vkGetDisplayPlaneSupportedDisplaysKHR>(instance->vkGetInstanceProcAddr("vkGetDisplayPlaneSupportedDisplaysKHR"));
  vkGetDisplayModePropertiesKHR_ = reinterpret_cast<PFN_vkGetDisplayModePropertiesKHR>(instance->vkGetInstanceProcAddr("vkGetDisplayModePropertiesKHR"));
  vkCreateDisplayModeKHR_ = reinterpret_cast<PFN_vkCreateDisplayModeKHR>(instance->vkGetInstanceProcAddr("vkCreateDisplayModeKHR"));
  vkGetDisplayPlaneCapabilitiesKHR_ = reinterpret_cast<PFN_vkGetDisplayPlaneCapabilitiesKHR>(instance->vkGetInstanceProcAddr("vkGetDisplayPlaneCapabilitiesKHR"));
#if defined(VK_USE_PLATFORM_XLIB_KHR)
  vkGetPhysicalDeviceXlibPresentationSupportKHR_ = reinterpret_cast<PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR>(instance->vkGetInstanceProcAddr("vkGetPhysicalDeviceXlibPresentationSupportKHR"));
#endif
#if defined(VK_USE_PLATFORM_XCB_KHR)
  vkGetPhysicalDeviceXcbPresentationSupportKHR_ = reinterpret_cast<PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR>(instance->vkGetInstanceProcAddr("vkGetPhysicalDeviceXcbPresentationSupportKHR"));
#endif
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
  vkGetPhysicalDeviceWaylandPresentationSupportKHR_ = reinterpret_cast<PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR>(instance->vkGetInstanceProcAddr("vkGetPhysicalDeviceWaylandPresentationSupportKHR"));
#endif
#if defined(VK_USE_PLATFORM_MIR_KHR)
  vkGetPhysicalDeviceMirPresentationSupportKHR_ = reinterpret_cast<PFN_vkGetPhysicalDeviceMirPresentationSupportKHR>(instance->vkGetInstanceProcAddr("vkGetPhysicalDeviceMirPresentationSupportKHR"));
#endif
#if defined(VK_USE_PLATFORM_WIN32_KHR)
  vkGetPhysicalDeviceWin32PresentationSupportKHR_ = reinterpret_cast<PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR>(instance->vkGetInstanceProcAddr("vkGetPhysicalDeviceWin32PresentationSupportKHR"));
#endif
  vkGetPhysicalDeviceExternalImageFormatPropertiesNV_ = reinterpret_cast<PFN_vkGetPhysicalDeviceExternalImageFormatPropertiesNV>(instance->vkGetInstanceProcAddr("vkGetPhysicalDeviceExternalImageFormatPropertiesNV"));
  vkGetPhysicalDeviceFeatures2KHR_ = reinterpret_cast<PFN_vkGetPhysicalDeviceFeatures2KHR>(instance->vkGetInstanceProcAddr("vkGetPhysicalDeviceFeatures2KHR"));
  vkGetPhysicalDeviceProperties2KHR_ = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties2KHR>(instance->vkGetInstanceProcAddr("vkGetPhysicalDeviceProperties2KHR"));
  vkGetPhysicalDeviceFormatProperties2KHR_ = reinterpret_cast<PFN_vkGetPhysicalDeviceFormatProperties2KHR>(instance->vkGetInstanceProcAddr("vkGetPhysicalDeviceFormatProperties2KHR"));
  vkGetPhysicalDeviceImageFormatProperties2KHR_ = reinterpret_cast<PFN_vkGetPhysicalDeviceImageFormatProperties2KHR>(instance->vkGetInstanceProcAddr("vkGetPhysicalDeviceImageFormatProperties2KHR"));
  vkGetPhysicalDeviceQueueFamilyProperties2KHR_ = reinterpret_cast<PFN_vkGetPhysicalDeviceQueueFamilyProperties2KHR>(instance->vkGetInstanceProcAddr("vkGetPhysicalDeviceQueueFamilyProperties2KHR"));
  vkGetPhysicalDeviceMemoryProperties2KHR_ = reinterpret_cast<PFN_vkGetPhysicalDeviceMemoryProperties2KHR>(instance->vkGetInstanceProcAddr("vkGetPhysicalDeviceMemoryProperties2KHR"));
  vkGetPhysicalDeviceSparseImageFormatProperties2KHR_ = reinterpret_cast<PFN_vkGetPhysicalDeviceSparseImageFormatProperties2KHR>(instance->vkGetInstanceProcAddr("vkGetPhysicalDeviceSparseImageFormatProperties2KHR"));
  vkGetPhysicalDeviceExternalBufferPropertiesKHR_ = reinterpret_cast<PFN_vkGetPhysicalDeviceExternalBufferPropertiesKHR>(instance->vkGetInstanceProcAddr("vkGetPhysicalDeviceExternalBufferPropertiesKHR"));
  vkGetPhysicalDeviceExternalSemaphorePropertiesKHR_ = reinterpret_cast<PFN_vkGetPhysicalDeviceExternalSemaphorePropertiesKHR>(instance->vkGetInstanceProcAddr("vkGetPhysicalDeviceExternalSemaphorePropertiesKHR"));
  vkGetPhysicalDeviceGeneratedCommandsPropertiesNVX_ = reinterpret_cast<PFN_vkGetPhysicalDeviceGeneratedCommandsPropertiesNVX>(instance->vkGetInstanceProcAddr("vkGetPhysicalDeviceGeneratedCommandsPropertiesNVX"));
  vkReleaseDisplayEXT_ = reinterpret_cast<PFN_vkReleaseDisplayEXT>(instance->vkGetInstanceProcAddr("vkReleaseDisplayEXT"));
#if defined(VK_USE_PLATFORM_XLIB_XRANDR_EXT)
  vkAcquireXlibDisplayEXT_ = reinterpret_cast<PFN_vkAcquireXlibDisplayEXT>(instance->vkGetInstanceProcAddr("vkAcquireXlibDisplayEXT"));
#endif
#if defined(VK_USE_PLATFORM_XLIB_XRANDR_EXT)
  vkGetRandROutputDisplayEXT_ = reinterpret_cast<PFN_vkGetRandROutputDisplayEXT>(instance->vkGetInstanceProcAddr("vkGetRandROutputDisplayEXT"));
#endif
  vkGetPhysicalDeviceSurfaceCapabilities2EXT_ = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceCapabilities2EXT>(instance->vkGetInstanceProcAddr("vkGetPhysicalDeviceSurfaceCapabilities2EXT"));
  vkGetPhysicalDeviceExternalFencePropertiesKHR_ = reinterpret_cast<PFN_vkGetPhysicalDeviceExternalFencePropertiesKHR>(instance->vkGetInstanceProcAddr("vkGetPhysicalDeviceExternalFencePropertiesKHR"));
  vkGetPhysicalDeviceSurfaceCapabilities2KHR_ = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR>(instance->vkGetInstanceProcAddr("vkGetPhysicalDeviceSurfaceCapabilities2KHR"));
  vkGetPhysicalDeviceSurfaceFormats2KHR_ = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceFormats2KHR>(instance->vkGetInstanceProcAddr("vkGetPhysicalDeviceSurfaceFormats2KHR"));
  vkGetPhysicalDeviceMultisamplePropertiesEXT_ = reinterpret_cast<PFN_vkGetPhysicalDeviceMultisamplePropertiesEXT>(instance->vkGetInstanceProcAddr("vkGetPhysicalDeviceMultisamplePropertiesEXT"));
}

/*
 * ------------------------------------------------------
 * DeviceFunctions
 * ------------------------------------------------------
*/

PFN_vkVoidFunction DeviceFunctions::vkGetDeviceProcAddr(const char* pName) const {
  return this->vkGetDeviceProcAddr_(device_, pName);
}

void DeviceFunctions::vkDestroyDevice(const VkAllocationCallbacks* pAllocator) const {
  return this->vkDestroyDevice_(device_, pAllocator);
}

void DeviceFunctions::vkGetDeviceQueue(uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue) const {
  return this->vkGetDeviceQueue_(device_, queueFamilyIndex, queueIndex, pQueue);
}

VkResult DeviceFunctions::vkDeviceWaitIdle() const {
  return this->vkDeviceWaitIdle_(device_);
}

VkResult DeviceFunctions::vkAllocateMemory(const VkMemoryAllocateInfo* pAllocateInfo, const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory) const {
  return this->vkAllocateMemory_(device_, pAllocateInfo, pAllocator, pMemory);
}

void DeviceFunctions::vkFreeMemory(VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator) const {
  return this->vkFreeMemory_(device_, memory, pAllocator);
}

VkResult DeviceFunctions::vkMapMemory(VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** ppData) const {
  return this->vkMapMemory_(device_, memory, offset, size, flags, ppData);
}

void DeviceFunctions::vkUnmapMemory(VkDeviceMemory memory) const {
  return this->vkUnmapMemory_(device_, memory);
}

VkResult DeviceFunctions::vkFlushMappedMemoryRanges(uint32_t memoryRangeCount, const VkMappedMemoryRange* pMemoryRanges) const {
  return this->vkFlushMappedMemoryRanges_(device_, memoryRangeCount, pMemoryRanges);
}

VkResult DeviceFunctions::vkInvalidateMappedMemoryRanges(uint32_t memoryRangeCount, const VkMappedMemoryRange* pMemoryRanges) const {
  return this->vkInvalidateMappedMemoryRanges_(device_, memoryRangeCount, pMemoryRanges);
}

void DeviceFunctions::vkGetDeviceMemoryCommitment(VkDeviceMemory memory, VkDeviceSize* pCommittedMemoryInBytes) const {
  return this->vkGetDeviceMemoryCommitment_(device_, memory, pCommittedMemoryInBytes);
}

VkResult DeviceFunctions::vkBindBufferMemory(VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset) const {
  return this->vkBindBufferMemory_(device_, buffer, memory, memoryOffset);
}

VkResult DeviceFunctions::vkBindImageMemory(VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset) const {
  return this->vkBindImageMemory_(device_, image, memory, memoryOffset);
}

void DeviceFunctions::vkGetBufferMemoryRequirements(VkBuffer buffer, VkMemoryRequirements* pMemoryRequirements) const {
  return this->vkGetBufferMemoryRequirements_(device_, buffer, pMemoryRequirements);
}

void DeviceFunctions::vkGetImageMemoryRequirements(VkImage image, VkMemoryRequirements* pMemoryRequirements) const {
  return this->vkGetImageMemoryRequirements_(device_, image, pMemoryRequirements);
}

void DeviceFunctions::vkGetImageSparseMemoryRequirements(VkImage image, uint32_t* pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements* pSparseMemoryRequirements) const {
  return this->vkGetImageSparseMemoryRequirements_(device_, image, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}

VkResult DeviceFunctions::vkCreateFence(const VkFenceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkFence* pFence) const {
  return this->vkCreateFence_(device_, pCreateInfo, pAllocator, pFence);
}

void DeviceFunctions::vkDestroyFence(VkFence fence, const VkAllocationCallbacks* pAllocator) const {
  return this->vkDestroyFence_(device_, fence, pAllocator);
}

VkResult DeviceFunctions::vkResetFences(uint32_t fenceCount, const VkFence* pFences) const {
  return this->vkResetFences_(device_, fenceCount, pFences);
}

VkResult DeviceFunctions::vkGetFenceStatus(VkFence fence) const {
  return this->vkGetFenceStatus_(device_, fence);
}

VkResult DeviceFunctions::vkWaitForFences(uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll, uint64_t timeout) const {
  return this->vkWaitForFences_(device_, fenceCount, pFences, waitAll, timeout);
}

VkResult DeviceFunctions::vkCreateSemaphore(const VkSemaphoreCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore) const {
  return this->vkCreateSemaphore_(device_, pCreateInfo, pAllocator, pSemaphore);
}

void DeviceFunctions::vkDestroySemaphore(VkSemaphore semaphore, const VkAllocationCallbacks* pAllocator) const {
  return this->vkDestroySemaphore_(device_, semaphore, pAllocator);
}

VkResult DeviceFunctions::vkCreateEvent(const VkEventCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkEvent* pEvent) const {
  return this->vkCreateEvent_(device_, pCreateInfo, pAllocator, pEvent);
}

void DeviceFunctions::vkDestroyEvent(VkEvent event, const VkAllocationCallbacks* pAllocator) const {
  return this->vkDestroyEvent_(device_, event, pAllocator);
}

VkResult DeviceFunctions::vkGetEventStatus(VkEvent event) const {
  return this->vkGetEventStatus_(device_, event);
}

VkResult DeviceFunctions::vkSetEvent(VkEvent event) const {
  return this->vkSetEvent_(device_, event);
}

VkResult DeviceFunctions::vkResetEvent(VkEvent event) const {
  return this->vkResetEvent_(device_, event);
}

VkResult DeviceFunctions::vkCreateQueryPool(const VkQueryPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkQueryPool* pQueryPool) const {
  return this->vkCreateQueryPool_(device_, pCreateInfo, pAllocator, pQueryPool);
}

void DeviceFunctions::vkDestroyQueryPool(VkQueryPool queryPool, const VkAllocationCallbacks* pAllocator) const {
  return this->vkDestroyQueryPool_(device_, queryPool, pAllocator);
}

VkResult DeviceFunctions::vkGetQueryPoolResults(VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, size_t dataSize, void* pData, VkDeviceSize stride, VkQueryResultFlags flags) const {
  return this->vkGetQueryPoolResults_(device_, queryPool, firstQuery, queryCount, dataSize, pData, stride, flags);
}

VkResult DeviceFunctions::vkCreateBuffer(const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer) const {
  return this->vkCreateBuffer_(device_, pCreateInfo, pAllocator, pBuffer);
}

void DeviceFunctions::vkDestroyBuffer(VkBuffer buffer, const VkAllocationCallbacks* pAllocator) const {
  return this->vkDestroyBuffer_(device_, buffer, pAllocator);
}

VkResult DeviceFunctions::vkCreateBufferView(const VkBufferViewCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBufferView* pView) const {
  return this->vkCreateBufferView_(device_, pCreateInfo, pAllocator, pView);
}

void DeviceFunctions::vkDestroyBufferView(VkBufferView bufferView, const VkAllocationCallbacks* pAllocator) const {
  return this->vkDestroyBufferView_(device_, bufferView, pAllocator);
}

VkResult DeviceFunctions::vkCreateImage(const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImage* pImage) const {
  return this->vkCreateImage_(device_, pCreateInfo, pAllocator, pImage);
}

void DeviceFunctions::vkDestroyImage(VkImage image, const VkAllocationCallbacks* pAllocator) const {
  return this->vkDestroyImage_(device_, image, pAllocator);
}

void DeviceFunctions::vkGetImageSubresourceLayout(VkImage image, const VkImageSubresource* pSubresource, VkSubresourceLayout* pLayout) const {
  return this->vkGetImageSubresourceLayout_(device_, image, pSubresource, pLayout);
}

VkResult DeviceFunctions::vkCreateImageView(const VkImageViewCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImageView* pView) const {
  return this->vkCreateImageView_(device_, pCreateInfo, pAllocator, pView);
}

void DeviceFunctions::vkDestroyImageView(VkImageView imageView, const VkAllocationCallbacks* pAllocator) const {
  return this->vkDestroyImageView_(device_, imageView, pAllocator);
}

VkResult DeviceFunctions::vkCreateShaderModule(const VkShaderModuleCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule) const {
  return this->vkCreateShaderModule_(device_, pCreateInfo, pAllocator, pShaderModule);
}

void DeviceFunctions::vkDestroyShaderModule(VkShaderModule shaderModule, const VkAllocationCallbacks* pAllocator) const {
  return this->vkDestroyShaderModule_(device_, shaderModule, pAllocator);
}

VkResult DeviceFunctions::vkCreatePipelineCache(const VkPipelineCacheCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineCache* pPipelineCache) const {
  return this->vkCreatePipelineCache_(device_, pCreateInfo, pAllocator, pPipelineCache);
}

void DeviceFunctions::vkDestroyPipelineCache(VkPipelineCache pipelineCache, const VkAllocationCallbacks* pAllocator) const {
  return this->vkDestroyPipelineCache_(device_, pipelineCache, pAllocator);
}

VkResult DeviceFunctions::vkGetPipelineCacheData(VkPipelineCache pipelineCache, size_t* pDataSize, void* pData) const {
  return this->vkGetPipelineCacheData_(device_, pipelineCache, pDataSize, pData);
}

VkResult DeviceFunctions::vkMergePipelineCaches(VkPipelineCache dstCache, uint32_t srcCacheCount, const VkPipelineCache* pSrcCaches) const {
  return this->vkMergePipelineCaches_(device_, dstCache, srcCacheCount, pSrcCaches);
}

VkResult DeviceFunctions::vkCreateGraphicsPipelines(VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkGraphicsPipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines) const {
  return this->vkCreateGraphicsPipelines_(device_, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
}

VkResult DeviceFunctions::vkCreateComputePipelines(VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkComputePipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines) const {
  return this->vkCreateComputePipelines_(device_, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
}

void DeviceFunctions::vkDestroyPipeline(VkPipeline pipeline, const VkAllocationCallbacks* pAllocator) const {
  return this->vkDestroyPipeline_(device_, pipeline, pAllocator);
}

VkResult DeviceFunctions::vkCreatePipelineLayout(const VkPipelineLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout) const {
  return this->vkCreatePipelineLayout_(device_, pCreateInfo, pAllocator, pPipelineLayout);
}

void DeviceFunctions::vkDestroyPipelineLayout(VkPipelineLayout pipelineLayout, const VkAllocationCallbacks* pAllocator) const {
  return this->vkDestroyPipelineLayout_(device_, pipelineLayout, pAllocator);
}

VkResult DeviceFunctions::vkCreateSampler(const VkSamplerCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSampler* pSampler) const {
  return this->vkCreateSampler_(device_, pCreateInfo, pAllocator, pSampler);
}

void DeviceFunctions::vkDestroySampler(VkSampler sampler, const VkAllocationCallbacks* pAllocator) const {
  return this->vkDestroySampler_(device_, sampler, pAllocator);
}

VkResult DeviceFunctions::vkCreateDescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorSetLayout* pSetLayout) const {
  return this->vkCreateDescriptorSetLayout_(device_, pCreateInfo, pAllocator, pSetLayout);
}

void DeviceFunctions::vkDestroyDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout, const VkAllocationCallbacks* pAllocator) const {
  return this->vkDestroyDescriptorSetLayout_(device_, descriptorSetLayout, pAllocator);
}

VkResult DeviceFunctions::vkCreateDescriptorPool(const VkDescriptorPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorPool* pDescriptorPool) const {
  return this->vkCreateDescriptorPool_(device_, pCreateInfo, pAllocator, pDescriptorPool);
}

void DeviceFunctions::vkDestroyDescriptorPool(VkDescriptorPool descriptorPool, const VkAllocationCallbacks* pAllocator) const {
  return this->vkDestroyDescriptorPool_(device_, descriptorPool, pAllocator);
}

VkResult DeviceFunctions::vkResetDescriptorPool(VkDescriptorPool descriptorPool, VkDescriptorPoolResetFlags flags) const {
  return this->vkResetDescriptorPool_(device_, descriptorPool, flags);
}

VkResult DeviceFunctions::vkAllocateDescriptorSets(const VkDescriptorSetAllocateInfo* pAllocateInfo, VkDescriptorSet* pDescriptorSets) const {
  return this->vkAllocateDescriptorSets_(device_, pAllocateInfo, pDescriptorSets);
}

VkResult DeviceFunctions::vkFreeDescriptorSets(VkDescriptorPool descriptorPool, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets) const {
  return this->vkFreeDescriptorSets_(device_, descriptorPool, descriptorSetCount, pDescriptorSets);
}

void DeviceFunctions::vkUpdateDescriptorSets(uint32_t descriptorWriteCount, const VkWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount, const VkCopyDescriptorSet* pDescriptorCopies) const {
  return this->vkUpdateDescriptorSets_(device_, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies);
}

VkResult DeviceFunctions::vkCreateFramebuffer(const VkFramebufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer) const {
  return this->vkCreateFramebuffer_(device_, pCreateInfo, pAllocator, pFramebuffer);
}

void DeviceFunctions::vkDestroyFramebuffer(VkFramebuffer framebuffer, const VkAllocationCallbacks* pAllocator) const {
  return this->vkDestroyFramebuffer_(device_, framebuffer, pAllocator);
}

VkResult DeviceFunctions::vkCreateRenderPass(const VkRenderPassCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass) const {
  return this->vkCreateRenderPass_(device_, pCreateInfo, pAllocator, pRenderPass);
}

void DeviceFunctions::vkDestroyRenderPass(VkRenderPass renderPass, const VkAllocationCallbacks* pAllocator) const {
  return this->vkDestroyRenderPass_(device_, renderPass, pAllocator);
}

void DeviceFunctions::vkGetRenderAreaGranularity(VkRenderPass renderPass, VkExtent2D* pGranularity) const {
  return this->vkGetRenderAreaGranularity_(device_, renderPass, pGranularity);
}

VkResult DeviceFunctions::vkCreateCommandPool(const VkCommandPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool) const {
  return this->vkCreateCommandPool_(device_, pCreateInfo, pAllocator, pCommandPool);
}

void DeviceFunctions::vkDestroyCommandPool(VkCommandPool commandPool, const VkAllocationCallbacks* pAllocator) const {
  return this->vkDestroyCommandPool_(device_, commandPool, pAllocator);
}

VkResult DeviceFunctions::vkResetCommandPool(VkCommandPool commandPool, VkCommandPoolResetFlags flags) const {
  return this->vkResetCommandPool_(device_, commandPool, flags);
}

VkResult DeviceFunctions::vkAllocateCommandBuffers(const VkCommandBufferAllocateInfo* pAllocateInfo, VkCommandBuffer* pCommandBuffers) const {
  return this->vkAllocateCommandBuffers_(device_, pAllocateInfo, pCommandBuffers);
}

void DeviceFunctions::vkFreeCommandBuffers(VkCommandPool commandPool, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers) const {
  return this->vkFreeCommandBuffers_(device_, commandPool, commandBufferCount, pCommandBuffers);
}

VkResult DeviceFunctions::vkCreateSwapchainKHR(const VkSwapchainCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain) const {
  return this->vkCreateSwapchainKHR_(device_, pCreateInfo, pAllocator, pSwapchain);
}

void DeviceFunctions::vkDestroySwapchainKHR(VkSwapchainKHR swapchain, const VkAllocationCallbacks* pAllocator) const {
  return this->vkDestroySwapchainKHR_(device_, swapchain, pAllocator);
}

VkResult DeviceFunctions::vkGetSwapchainImagesKHR(VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount, VkImage* pSwapchainImages) const {
  return this->vkGetSwapchainImagesKHR_(device_, swapchain, pSwapchainImageCount, pSwapchainImages);
}

VkResult DeviceFunctions::vkAcquireNextImageKHR(VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex) const {
  return this->vkAcquireNextImageKHR_(device_, swapchain, timeout, semaphore, fence, pImageIndex);
}

VkResult DeviceFunctions::vkCreateSharedSwapchainsKHR(uint32_t swapchainCount, const VkSwapchainCreateInfoKHR* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchains) const {
  return this->vkCreateSharedSwapchainsKHR_(device_, swapchainCount, pCreateInfos, pAllocator, pSwapchains);
}

VkResult DeviceFunctions::vkDebugMarkerSetObjectTagEXT(const VkDebugMarkerObjectTagInfoEXT* pTagInfo) const {
  return this->vkDebugMarkerSetObjectTagEXT_(device_, pTagInfo);
}

VkResult DeviceFunctions::vkDebugMarkerSetObjectNameEXT(const VkDebugMarkerObjectNameInfoEXT* pNameInfo) const {
  return this->vkDebugMarkerSetObjectNameEXT_(device_, pNameInfo);
}

VkResult DeviceFunctions::vkGetShaderInfoAMD(VkPipeline pipeline, VkShaderStageFlagBits shaderStage, VkShaderInfoTypeAMD infoType, size_t* pInfoSize, void* pInfo) const {
  return this->vkGetShaderInfoAMD_(device_, pipeline, shaderStage, infoType, pInfoSize, pInfo);
}

#if defined(VK_USE_PLATFORM_WIN32_KHR)
VkResult DeviceFunctions::vkGetMemoryWin32HandleNV(VkDeviceMemory memory, VkExternalMemoryHandleTypeFlagsNV handleType, HANDLE* pHandle) const {
  return this->vkGetMemoryWin32HandleNV_(device_, memory, handleType, pHandle);
}
#endif

void DeviceFunctions::vkGetDeviceGroupPeerMemoryFeaturesKHX(uint32_t heapIndex, uint32_t localDeviceIndex, uint32_t remoteDeviceIndex, VkPeerMemoryFeatureFlagsKHX* pPeerMemoryFeatures) const {
  return this->vkGetDeviceGroupPeerMemoryFeaturesKHX_(device_, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
}

void DeviceFunctions::vkTrimCommandPoolKHR(VkCommandPool commandPool, VkCommandPoolTrimFlagsKHR flags) const {
  return this->vkTrimCommandPoolKHR_(device_, commandPool, flags);
}

#if defined(VK_USE_PLATFORM_WIN32_KHR)
VkResult DeviceFunctions::vkGetMemoryWin32HandleKHR(const VkMemoryGetWin32HandleInfoKHR* pGetWin32HandleInfo, HANDLE* pHandle) const {
  return this->vkGetMemoryWin32HandleKHR_(device_, pGetWin32HandleInfo, pHandle);
}
#endif

#if defined(VK_USE_PLATFORM_WIN32_KHR)
VkResult DeviceFunctions::vkGetMemoryWin32HandlePropertiesKHR(VkExternalMemoryHandleTypeFlagBitsKHR handleType, HANDLE handle, VkMemoryWin32HandlePropertiesKHR* pMemoryWin32HandleProperties) const {
  return this->vkGetMemoryWin32HandlePropertiesKHR_(device_, handleType, handle, pMemoryWin32HandleProperties);
}
#endif

VkResult DeviceFunctions::vkGetMemoryFdKHR(const VkMemoryGetFdInfoKHR* pGetFdInfo, int* pFd) const {
  return this->vkGetMemoryFdKHR_(device_, pGetFdInfo, pFd);
}

VkResult DeviceFunctions::vkGetMemoryFdPropertiesKHR(VkExternalMemoryHandleTypeFlagBitsKHR handleType, int fd, VkMemoryFdPropertiesKHR* pMemoryFdProperties) const {
  return this->vkGetMemoryFdPropertiesKHR_(device_, handleType, fd, pMemoryFdProperties);
}

#if defined(VK_USE_PLATFORM_WIN32_KHR)
VkResult DeviceFunctions::vkImportSemaphoreWin32HandleKHR(const VkImportSemaphoreWin32HandleInfoKHR* pImportSemaphoreWin32HandleInfo) const {
  return this->vkImportSemaphoreWin32HandleKHR_(device_, pImportSemaphoreWin32HandleInfo);
}
#endif

#if defined(VK_USE_PLATFORM_WIN32_KHR)
VkResult DeviceFunctions::vkGetSemaphoreWin32HandleKHR(const VkSemaphoreGetWin32HandleInfoKHR* pGetWin32HandleInfo, HANDLE* pHandle) const {
  return this->vkGetSemaphoreWin32HandleKHR_(device_, pGetWin32HandleInfo, pHandle);
}
#endif

VkResult DeviceFunctions::vkImportSemaphoreFdKHR(const VkImportSemaphoreFdInfoKHR* pImportSemaphoreFdInfo) const {
  return this->vkImportSemaphoreFdKHR_(device_, pImportSemaphoreFdInfo);
}

VkResult DeviceFunctions::vkGetSemaphoreFdKHR(const VkSemaphoreGetFdInfoKHR* pGetFdInfo, int* pFd) const {
  return this->vkGetSemaphoreFdKHR_(device_, pGetFdInfo, pFd);
}

VkResult DeviceFunctions::vkCreateDescriptorUpdateTemplateKHR(const VkDescriptorUpdateTemplateCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorUpdateTemplateKHR* pDescriptorUpdateTemplate) const {
  return this->vkCreateDescriptorUpdateTemplateKHR_(device_, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);
}

void DeviceFunctions::vkDestroyDescriptorUpdateTemplateKHR(VkDescriptorUpdateTemplateKHR descriptorUpdateTemplate, const VkAllocationCallbacks* pAllocator) const {
  return this->vkDestroyDescriptorUpdateTemplateKHR_(device_, descriptorUpdateTemplate, pAllocator);
}

void DeviceFunctions::vkUpdateDescriptorSetWithTemplateKHR(VkDescriptorSet descriptorSet, VkDescriptorUpdateTemplateKHR descriptorUpdateTemplate, const void* pData) const {
  return this->vkUpdateDescriptorSetWithTemplateKHR_(device_, descriptorSet, descriptorUpdateTemplate, pData);
}

VkResult DeviceFunctions::vkCreateIndirectCommandsLayoutNVX(const VkIndirectCommandsLayoutCreateInfoNVX* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkIndirectCommandsLayoutNVX* pIndirectCommandsLayout) const {
  return this->vkCreateIndirectCommandsLayoutNVX_(device_, pCreateInfo, pAllocator, pIndirectCommandsLayout);
}

void DeviceFunctions::vkDestroyIndirectCommandsLayoutNVX(VkIndirectCommandsLayoutNVX indirectCommandsLayout, const VkAllocationCallbacks* pAllocator) const {
  return this->vkDestroyIndirectCommandsLayoutNVX_(device_, indirectCommandsLayout, pAllocator);
}

VkResult DeviceFunctions::vkCreateObjectTableNVX(const VkObjectTableCreateInfoNVX* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkObjectTableNVX* pObjectTable) const {
  return this->vkCreateObjectTableNVX_(device_, pCreateInfo, pAllocator, pObjectTable);
}

void DeviceFunctions::vkDestroyObjectTableNVX(VkObjectTableNVX objectTable, const VkAllocationCallbacks* pAllocator) const {
  return this->vkDestroyObjectTableNVX_(device_, objectTable, pAllocator);
}

VkResult DeviceFunctions::vkRegisterObjectsNVX(VkObjectTableNVX objectTable, uint32_t objectCount, const VkObjectTableEntryNVX* const* ppObjectTableEntries, const uint32_t* pObjectIndices) const {
  return this->vkRegisterObjectsNVX_(device_, objectTable, objectCount, ppObjectTableEntries, pObjectIndices);
}

VkResult DeviceFunctions::vkUnregisterObjectsNVX(VkObjectTableNVX objectTable, uint32_t objectCount, const VkObjectEntryTypeNVX* pObjectEntryTypes, const uint32_t* pObjectIndices) const {
  return this->vkUnregisterObjectsNVX_(device_, objectTable, objectCount, pObjectEntryTypes, pObjectIndices);
}

VkResult DeviceFunctions::vkDisplayPowerControlEXT(VkDisplayKHR display, const VkDisplayPowerInfoEXT* pDisplayPowerInfo) const {
  return this->vkDisplayPowerControlEXT_(device_, display, pDisplayPowerInfo);
}

VkResult DeviceFunctions::vkRegisterDeviceEventEXT(const VkDeviceEventInfoEXT* pDeviceEventInfo, const VkAllocationCallbacks* pAllocator, VkFence* pFence) const {
  return this->vkRegisterDeviceEventEXT_(device_, pDeviceEventInfo, pAllocator, pFence);
}

VkResult DeviceFunctions::vkRegisterDisplayEventEXT(VkDisplayKHR display, const VkDisplayEventInfoEXT* pDisplayEventInfo, const VkAllocationCallbacks* pAllocator, VkFence* pFence) const {
  return this->vkRegisterDisplayEventEXT_(device_, display, pDisplayEventInfo, pAllocator, pFence);
}

VkResult DeviceFunctions::vkGetSwapchainCounterEXT(VkSwapchainKHR swapchain, VkSurfaceCounterFlagBitsEXT counter, uint64_t* pCounterValue) const {
  return this->vkGetSwapchainCounterEXT_(device_, swapchain, counter, pCounterValue);
}

VkResult DeviceFunctions::vkGetRefreshCycleDurationGOOGLE(VkSwapchainKHR swapchain, VkRefreshCycleDurationGOOGLE* pDisplayTimingProperties) const {
  return this->vkGetRefreshCycleDurationGOOGLE_(device_, swapchain, pDisplayTimingProperties);
}

VkResult DeviceFunctions::vkGetPastPresentationTimingGOOGLE(VkSwapchainKHR swapchain, uint32_t* pPresentationTimingCount, VkPastPresentationTimingGOOGLE* pPresentationTimings) const {
  return this->vkGetPastPresentationTimingGOOGLE_(device_, swapchain, pPresentationTimingCount, pPresentationTimings);
}

void DeviceFunctions::vkSetHdrMetadataEXT(uint32_t swapchainCount, const VkSwapchainKHR* pSwapchains, const VkHdrMetadataEXT* pMetadata) const {
  return this->vkSetHdrMetadataEXT_(device_, swapchainCount, pSwapchains, pMetadata);
}

VkResult DeviceFunctions::vkGetSwapchainStatusKHR(VkSwapchainKHR swapchain) const {
  return this->vkGetSwapchainStatusKHR_(device_, swapchain);
}

#if defined(VK_USE_PLATFORM_WIN32_KHR)
VkResult DeviceFunctions::vkImportFenceWin32HandleKHR(const VkImportFenceWin32HandleInfoKHR* pImportFenceWin32HandleInfo) const {
  return this->vkImportFenceWin32HandleKHR_(device_, pImportFenceWin32HandleInfo);
}
#endif

#if defined(VK_USE_PLATFORM_WIN32_KHR)
VkResult DeviceFunctions::vkGetFenceWin32HandleKHR(const VkFenceGetWin32HandleInfoKHR* pGetWin32HandleInfo, HANDLE* pHandle) const {
  return this->vkGetFenceWin32HandleKHR_(device_, pGetWin32HandleInfo, pHandle);
}
#endif

VkResult DeviceFunctions::vkImportFenceFdKHR(const VkImportFenceFdInfoKHR* pImportFenceFdInfo) const {
  return this->vkImportFenceFdKHR_(device_, pImportFenceFdInfo);
}

VkResult DeviceFunctions::vkGetFenceFdKHR(const VkFenceGetFdInfoKHR* pGetFdInfo, int* pFd) const {
  return this->vkGetFenceFdKHR_(device_, pGetFdInfo, pFd);
}

void DeviceFunctions::vkGetImageMemoryRequirements2KHR(const VkImageMemoryRequirementsInfo2KHR* pInfo, VkMemoryRequirements2KHR* pMemoryRequirements) const {
  return this->vkGetImageMemoryRequirements2KHR_(device_, pInfo, pMemoryRequirements);
}

void DeviceFunctions::vkGetBufferMemoryRequirements2KHR(const VkBufferMemoryRequirementsInfo2KHR* pInfo, VkMemoryRequirements2KHR* pMemoryRequirements) const {
  return this->vkGetBufferMemoryRequirements2KHR_(device_, pInfo, pMemoryRequirements);
}

void DeviceFunctions::vkGetImageSparseMemoryRequirements2KHR(const VkImageSparseMemoryRequirementsInfo2KHR* pInfo, uint32_t* pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements2KHR* pSparseMemoryRequirements) const {
  return this->vkGetImageSparseMemoryRequirements2KHR_(device_, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}

VkResult DeviceFunctions::vkCreateSamplerYcbcrConversionKHR(const VkSamplerYcbcrConversionCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSamplerYcbcrConversionKHR* pYcbcrConversion) const {
  return this->vkCreateSamplerYcbcrConversionKHR_(device_, pCreateInfo, pAllocator, pYcbcrConversion);
}

void DeviceFunctions::vkDestroySamplerYcbcrConversionKHR(VkSamplerYcbcrConversionKHR ycbcrConversion, const VkAllocationCallbacks* pAllocator) const {
  return this->vkDestroySamplerYcbcrConversionKHR_(device_, ycbcrConversion, pAllocator);
}

VkResult DeviceFunctions::vkBindBufferMemory2KHR(uint32_t bindInfoCount, const VkBindBufferMemoryInfoKHR* pBindInfos) const {
  return this->vkBindBufferMemory2KHR_(device_, bindInfoCount, pBindInfos);
}

VkResult DeviceFunctions::vkBindImageMemory2KHR(uint32_t bindInfoCount, const VkBindImageMemoryInfoKHR* pBindInfos) const {
  return this->vkBindImageMemory2KHR_(device_, bindInfoCount, pBindInfos);
}

VkResult DeviceFunctions::vkCreateValidationCacheEXT(const VkValidationCacheCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkValidationCacheEXT* pValidationCache) const {
  return this->vkCreateValidationCacheEXT_(device_, pCreateInfo, pAllocator, pValidationCache);
}

void DeviceFunctions::vkDestroyValidationCacheEXT(VkValidationCacheEXT validationCache, const VkAllocationCallbacks* pAllocator) const {
  return this->vkDestroyValidationCacheEXT_(device_, validationCache, pAllocator);
}

VkResult DeviceFunctions::vkMergeValidationCachesEXT(VkValidationCacheEXT dstCache, uint32_t srcCacheCount, const VkValidationCacheEXT* pSrcCaches) const {
  return this->vkMergeValidationCachesEXT_(device_, dstCache, srcCacheCount, pSrcCaches);
}

VkResult DeviceFunctions::vkGetValidationCacheDataEXT(VkValidationCacheEXT validationCache, size_t* pDataSize, void* pData) const {
  return this->vkGetValidationCacheDataEXT_(device_, validationCache, pDataSize, pData);
}

DeviceFunctions::DeviceFunctions(VkDevice device, InstanceFunctions* instance) {
  device_ = device;
  vkGetDeviceProcAddr_ = reinterpret_cast<PFN_vkGetDeviceProcAddr>(instance->vkGetInstanceProcAddr("vkGetDeviceProcAddr"));
  if (!vkGetDeviceProcAddr_) {
    throw VulkanProcNotFound("vkGetDeviceProcAddr");
  }
  vkDestroyDevice_ = reinterpret_cast<PFN_vkDestroyDevice>(this->vkGetDeviceProcAddr("vkDestroyDevice"));
  if (!vkDestroyDevice_) {
    throw VulkanProcNotFound("vkDestroyDevice");
  }
  vkGetDeviceQueue_ = reinterpret_cast<PFN_vkGetDeviceQueue>(this->vkGetDeviceProcAddr("vkGetDeviceQueue"));
  if (!vkGetDeviceQueue_) {
    throw VulkanProcNotFound("vkGetDeviceQueue");
  }
  vkDeviceWaitIdle_ = reinterpret_cast<PFN_vkDeviceWaitIdle>(this->vkGetDeviceProcAddr("vkDeviceWaitIdle"));
  if (!vkDeviceWaitIdle_) {
    throw VulkanProcNotFound("vkDeviceWaitIdle");
  }
  vkAllocateMemory_ = reinterpret_cast<PFN_vkAllocateMemory>(this->vkGetDeviceProcAddr("vkAllocateMemory"));
  if (!vkAllocateMemory_) {
    throw VulkanProcNotFound("vkAllocateMemory");
  }
  vkFreeMemory_ = reinterpret_cast<PFN_vkFreeMemory>(this->vkGetDeviceProcAddr("vkFreeMemory"));
  if (!vkFreeMemory_) {
    throw VulkanProcNotFound("vkFreeMemory");
  }
  vkMapMemory_ = reinterpret_cast<PFN_vkMapMemory>(this->vkGetDeviceProcAddr("vkMapMemory"));
  if (!vkMapMemory_) {
    throw VulkanProcNotFound("vkMapMemory");
  }
  vkUnmapMemory_ = reinterpret_cast<PFN_vkUnmapMemory>(this->vkGetDeviceProcAddr("vkUnmapMemory"));
  if (!vkUnmapMemory_) {
    throw VulkanProcNotFound("vkUnmapMemory");
  }
  vkFlushMappedMemoryRanges_ = reinterpret_cast<PFN_vkFlushMappedMemoryRanges>(this->vkGetDeviceProcAddr("vkFlushMappedMemoryRanges"));
  if (!vkFlushMappedMemoryRanges_) {
    throw VulkanProcNotFound("vkFlushMappedMemoryRanges");
  }
  vkInvalidateMappedMemoryRanges_ = reinterpret_cast<PFN_vkInvalidateMappedMemoryRanges>(this->vkGetDeviceProcAddr("vkInvalidateMappedMemoryRanges"));
  if (!vkInvalidateMappedMemoryRanges_) {
    throw VulkanProcNotFound("vkInvalidateMappedMemoryRanges");
  }
  vkGetDeviceMemoryCommitment_ = reinterpret_cast<PFN_vkGetDeviceMemoryCommitment>(this->vkGetDeviceProcAddr("vkGetDeviceMemoryCommitment"));
  if (!vkGetDeviceMemoryCommitment_) {
    throw VulkanProcNotFound("vkGetDeviceMemoryCommitment");
  }
  vkBindBufferMemory_ = reinterpret_cast<PFN_vkBindBufferMemory>(this->vkGetDeviceProcAddr("vkBindBufferMemory"));
  if (!vkBindBufferMemory_) {
    throw VulkanProcNotFound("vkBindBufferMemory");
  }
  vkBindImageMemory_ = reinterpret_cast<PFN_vkBindImageMemory>(this->vkGetDeviceProcAddr("vkBindImageMemory"));
  if (!vkBindImageMemory_) {
    throw VulkanProcNotFound("vkBindImageMemory");
  }
  vkGetBufferMemoryRequirements_ = reinterpret_cast<PFN_vkGetBufferMemoryRequirements>(this->vkGetDeviceProcAddr("vkGetBufferMemoryRequirements"));
  if (!vkGetBufferMemoryRequirements_) {
    throw VulkanProcNotFound("vkGetBufferMemoryRequirements");
  }
  vkGetImageMemoryRequirements_ = reinterpret_cast<PFN_vkGetImageMemoryRequirements>(this->vkGetDeviceProcAddr("vkGetImageMemoryRequirements"));
  if (!vkGetImageMemoryRequirements_) {
    throw VulkanProcNotFound("vkGetImageMemoryRequirements");
  }
  vkGetImageSparseMemoryRequirements_ = reinterpret_cast<PFN_vkGetImageSparseMemoryRequirements>(this->vkGetDeviceProcAddr("vkGetImageSparseMemoryRequirements"));
  if (!vkGetImageSparseMemoryRequirements_) {
    throw VulkanProcNotFound("vkGetImageSparseMemoryRequirements");
  }
  vkCreateFence_ = reinterpret_cast<PFN_vkCreateFence>(this->vkGetDeviceProcAddr("vkCreateFence"));
  if (!vkCreateFence_) {
    throw VulkanProcNotFound("vkCreateFence");
  }
  vkDestroyFence_ = reinterpret_cast<PFN_vkDestroyFence>(this->vkGetDeviceProcAddr("vkDestroyFence"));
  if (!vkDestroyFence_) {
    throw VulkanProcNotFound("vkDestroyFence");
  }
  vkResetFences_ = reinterpret_cast<PFN_vkResetFences>(this->vkGetDeviceProcAddr("vkResetFences"));
  if (!vkResetFences_) {
    throw VulkanProcNotFound("vkResetFences");
  }
  vkGetFenceStatus_ = reinterpret_cast<PFN_vkGetFenceStatus>(this->vkGetDeviceProcAddr("vkGetFenceStatus"));
  if (!vkGetFenceStatus_) {
    throw VulkanProcNotFound("vkGetFenceStatus");
  }
  vkWaitForFences_ = reinterpret_cast<PFN_vkWaitForFences>(this->vkGetDeviceProcAddr("vkWaitForFences"));
  if (!vkWaitForFences_) {
    throw VulkanProcNotFound("vkWaitForFences");
  }
  vkCreateSemaphore_ = reinterpret_cast<PFN_vkCreateSemaphore>(this->vkGetDeviceProcAddr("vkCreateSemaphore"));
  if (!vkCreateSemaphore_) {
    throw VulkanProcNotFound("vkCreateSemaphore");
  }
  vkDestroySemaphore_ = reinterpret_cast<PFN_vkDestroySemaphore>(this->vkGetDeviceProcAddr("vkDestroySemaphore"));
  if (!vkDestroySemaphore_) {
    throw VulkanProcNotFound("vkDestroySemaphore");
  }
  vkCreateEvent_ = reinterpret_cast<PFN_vkCreateEvent>(this->vkGetDeviceProcAddr("vkCreateEvent"));
  if (!vkCreateEvent_) {
    throw VulkanProcNotFound("vkCreateEvent");
  }
  vkDestroyEvent_ = reinterpret_cast<PFN_vkDestroyEvent>(this->vkGetDeviceProcAddr("vkDestroyEvent"));
  if (!vkDestroyEvent_) {
    throw VulkanProcNotFound("vkDestroyEvent");
  }
  vkGetEventStatus_ = reinterpret_cast<PFN_vkGetEventStatus>(this->vkGetDeviceProcAddr("vkGetEventStatus"));
  if (!vkGetEventStatus_) {
    throw VulkanProcNotFound("vkGetEventStatus");
  }
  vkSetEvent_ = reinterpret_cast<PFN_vkSetEvent>(this->vkGetDeviceProcAddr("vkSetEvent"));
  if (!vkSetEvent_) {
    throw VulkanProcNotFound("vkSetEvent");
  }
  vkResetEvent_ = reinterpret_cast<PFN_vkResetEvent>(this->vkGetDeviceProcAddr("vkResetEvent"));
  if (!vkResetEvent_) {
    throw VulkanProcNotFound("vkResetEvent");
  }
  vkCreateQueryPool_ = reinterpret_cast<PFN_vkCreateQueryPool>(this->vkGetDeviceProcAddr("vkCreateQueryPool"));
  if (!vkCreateQueryPool_) {
    throw VulkanProcNotFound("vkCreateQueryPool");
  }
  vkDestroyQueryPool_ = reinterpret_cast<PFN_vkDestroyQueryPool>(this->vkGetDeviceProcAddr("vkDestroyQueryPool"));
  if (!vkDestroyQueryPool_) {
    throw VulkanProcNotFound("vkDestroyQueryPool");
  }
  vkGetQueryPoolResults_ = reinterpret_cast<PFN_vkGetQueryPoolResults>(this->vkGetDeviceProcAddr("vkGetQueryPoolResults"));
  if (!vkGetQueryPoolResults_) {
    throw VulkanProcNotFound("vkGetQueryPoolResults");
  }
  vkCreateBuffer_ = reinterpret_cast<PFN_vkCreateBuffer>(this->vkGetDeviceProcAddr("vkCreateBuffer"));
  if (!vkCreateBuffer_) {
    throw VulkanProcNotFound("vkCreateBuffer");
  }
  vkDestroyBuffer_ = reinterpret_cast<PFN_vkDestroyBuffer>(this->vkGetDeviceProcAddr("vkDestroyBuffer"));
  if (!vkDestroyBuffer_) {
    throw VulkanProcNotFound("vkDestroyBuffer");
  }
  vkCreateBufferView_ = reinterpret_cast<PFN_vkCreateBufferView>(this->vkGetDeviceProcAddr("vkCreateBufferView"));
  if (!vkCreateBufferView_) {
    throw VulkanProcNotFound("vkCreateBufferView");
  }
  vkDestroyBufferView_ = reinterpret_cast<PFN_vkDestroyBufferView>(this->vkGetDeviceProcAddr("vkDestroyBufferView"));
  if (!vkDestroyBufferView_) {
    throw VulkanProcNotFound("vkDestroyBufferView");
  }
  vkCreateImage_ = reinterpret_cast<PFN_vkCreateImage>(this->vkGetDeviceProcAddr("vkCreateImage"));
  if (!vkCreateImage_) {
    throw VulkanProcNotFound("vkCreateImage");
  }
  vkDestroyImage_ = reinterpret_cast<PFN_vkDestroyImage>(this->vkGetDeviceProcAddr("vkDestroyImage"));
  if (!vkDestroyImage_) {
    throw VulkanProcNotFound("vkDestroyImage");
  }
  vkGetImageSubresourceLayout_ = reinterpret_cast<PFN_vkGetImageSubresourceLayout>(this->vkGetDeviceProcAddr("vkGetImageSubresourceLayout"));
  if (!vkGetImageSubresourceLayout_) {
    throw VulkanProcNotFound("vkGetImageSubresourceLayout");
  }
  vkCreateImageView_ = reinterpret_cast<PFN_vkCreateImageView>(this->vkGetDeviceProcAddr("vkCreateImageView"));
  if (!vkCreateImageView_) {
    throw VulkanProcNotFound("vkCreateImageView");
  }
  vkDestroyImageView_ = reinterpret_cast<PFN_vkDestroyImageView>(this->vkGetDeviceProcAddr("vkDestroyImageView"));
  if (!vkDestroyImageView_) {
    throw VulkanProcNotFound("vkDestroyImageView");
  }
  vkCreateShaderModule_ = reinterpret_cast<PFN_vkCreateShaderModule>(this->vkGetDeviceProcAddr("vkCreateShaderModule"));
  if (!vkCreateShaderModule_) {
    throw VulkanProcNotFound("vkCreateShaderModule");
  }
  vkDestroyShaderModule_ = reinterpret_cast<PFN_vkDestroyShaderModule>(this->vkGetDeviceProcAddr("vkDestroyShaderModule"));
  if (!vkDestroyShaderModule_) {
    throw VulkanProcNotFound("vkDestroyShaderModule");
  }
  vkCreatePipelineCache_ = reinterpret_cast<PFN_vkCreatePipelineCache>(this->vkGetDeviceProcAddr("vkCreatePipelineCache"));
  if (!vkCreatePipelineCache_) {
    throw VulkanProcNotFound("vkCreatePipelineCache");
  }
  vkDestroyPipelineCache_ = reinterpret_cast<PFN_vkDestroyPipelineCache>(this->vkGetDeviceProcAddr("vkDestroyPipelineCache"));
  if (!vkDestroyPipelineCache_) {
    throw VulkanProcNotFound("vkDestroyPipelineCache");
  }
  vkGetPipelineCacheData_ = reinterpret_cast<PFN_vkGetPipelineCacheData>(this->vkGetDeviceProcAddr("vkGetPipelineCacheData"));
  if (!vkGetPipelineCacheData_) {
    throw VulkanProcNotFound("vkGetPipelineCacheData");
  }
  vkMergePipelineCaches_ = reinterpret_cast<PFN_vkMergePipelineCaches>(this->vkGetDeviceProcAddr("vkMergePipelineCaches"));
  if (!vkMergePipelineCaches_) {
    throw VulkanProcNotFound("vkMergePipelineCaches");
  }
  vkCreateGraphicsPipelines_ = reinterpret_cast<PFN_vkCreateGraphicsPipelines>(this->vkGetDeviceProcAddr("vkCreateGraphicsPipelines"));
  if (!vkCreateGraphicsPipelines_) {
    throw VulkanProcNotFound("vkCreateGraphicsPipelines");
  }
  vkCreateComputePipelines_ = reinterpret_cast<PFN_vkCreateComputePipelines>(this->vkGetDeviceProcAddr("vkCreateComputePipelines"));
  if (!vkCreateComputePipelines_) {
    throw VulkanProcNotFound("vkCreateComputePipelines");
  }
  vkDestroyPipeline_ = reinterpret_cast<PFN_vkDestroyPipeline>(this->vkGetDeviceProcAddr("vkDestroyPipeline"));
  if (!vkDestroyPipeline_) {
    throw VulkanProcNotFound("vkDestroyPipeline");
  }
  vkCreatePipelineLayout_ = reinterpret_cast<PFN_vkCreatePipelineLayout>(this->vkGetDeviceProcAddr("vkCreatePipelineLayout"));
  if (!vkCreatePipelineLayout_) {
    throw VulkanProcNotFound("vkCreatePipelineLayout");
  }
  vkDestroyPipelineLayout_ = reinterpret_cast<PFN_vkDestroyPipelineLayout>(this->vkGetDeviceProcAddr("vkDestroyPipelineLayout"));
  if (!vkDestroyPipelineLayout_) {
    throw VulkanProcNotFound("vkDestroyPipelineLayout");
  }
  vkCreateSampler_ = reinterpret_cast<PFN_vkCreateSampler>(this->vkGetDeviceProcAddr("vkCreateSampler"));
  if (!vkCreateSampler_) {
    throw VulkanProcNotFound("vkCreateSampler");
  }
  vkDestroySampler_ = reinterpret_cast<PFN_vkDestroySampler>(this->vkGetDeviceProcAddr("vkDestroySampler"));
  if (!vkDestroySampler_) {
    throw VulkanProcNotFound("vkDestroySampler");
  }
  vkCreateDescriptorSetLayout_ = reinterpret_cast<PFN_vkCreateDescriptorSetLayout>(this->vkGetDeviceProcAddr("vkCreateDescriptorSetLayout"));
  if (!vkCreateDescriptorSetLayout_) {
    throw VulkanProcNotFound("vkCreateDescriptorSetLayout");
  }
  vkDestroyDescriptorSetLayout_ = reinterpret_cast<PFN_vkDestroyDescriptorSetLayout>(this->vkGetDeviceProcAddr("vkDestroyDescriptorSetLayout"));
  if (!vkDestroyDescriptorSetLayout_) {
    throw VulkanProcNotFound("vkDestroyDescriptorSetLayout");
  }
  vkCreateDescriptorPool_ = reinterpret_cast<PFN_vkCreateDescriptorPool>(this->vkGetDeviceProcAddr("vkCreateDescriptorPool"));
  if (!vkCreateDescriptorPool_) {
    throw VulkanProcNotFound("vkCreateDescriptorPool");
  }
  vkDestroyDescriptorPool_ = reinterpret_cast<PFN_vkDestroyDescriptorPool>(this->vkGetDeviceProcAddr("vkDestroyDescriptorPool"));
  if (!vkDestroyDescriptorPool_) {
    throw VulkanProcNotFound("vkDestroyDescriptorPool");
  }
  vkResetDescriptorPool_ = reinterpret_cast<PFN_vkResetDescriptorPool>(this->vkGetDeviceProcAddr("vkResetDescriptorPool"));
  if (!vkResetDescriptorPool_) {
    throw VulkanProcNotFound("vkResetDescriptorPool");
  }
  vkAllocateDescriptorSets_ = reinterpret_cast<PFN_vkAllocateDescriptorSets>(this->vkGetDeviceProcAddr("vkAllocateDescriptorSets"));
  if (!vkAllocateDescriptorSets_) {
    throw VulkanProcNotFound("vkAllocateDescriptorSets");
  }
  vkFreeDescriptorSets_ = reinterpret_cast<PFN_vkFreeDescriptorSets>(this->vkGetDeviceProcAddr("vkFreeDescriptorSets"));
  if (!vkFreeDescriptorSets_) {
    throw VulkanProcNotFound("vkFreeDescriptorSets");
  }
  vkUpdateDescriptorSets_ = reinterpret_cast<PFN_vkUpdateDescriptorSets>(this->vkGetDeviceProcAddr("vkUpdateDescriptorSets"));
  if (!vkUpdateDescriptorSets_) {
    throw VulkanProcNotFound("vkUpdateDescriptorSets");
  }
  vkCreateFramebuffer_ = reinterpret_cast<PFN_vkCreateFramebuffer>(this->vkGetDeviceProcAddr("vkCreateFramebuffer"));
  if (!vkCreateFramebuffer_) {
    throw VulkanProcNotFound("vkCreateFramebuffer");
  }
  vkDestroyFramebuffer_ = reinterpret_cast<PFN_vkDestroyFramebuffer>(this->vkGetDeviceProcAddr("vkDestroyFramebuffer"));
  if (!vkDestroyFramebuffer_) {
    throw VulkanProcNotFound("vkDestroyFramebuffer");
  }
  vkCreateRenderPass_ = reinterpret_cast<PFN_vkCreateRenderPass>(this->vkGetDeviceProcAddr("vkCreateRenderPass"));
  if (!vkCreateRenderPass_) {
    throw VulkanProcNotFound("vkCreateRenderPass");
  }
  vkDestroyRenderPass_ = reinterpret_cast<PFN_vkDestroyRenderPass>(this->vkGetDeviceProcAddr("vkDestroyRenderPass"));
  if (!vkDestroyRenderPass_) {
    throw VulkanProcNotFound("vkDestroyRenderPass");
  }
  vkGetRenderAreaGranularity_ = reinterpret_cast<PFN_vkGetRenderAreaGranularity>(this->vkGetDeviceProcAddr("vkGetRenderAreaGranularity"));
  if (!vkGetRenderAreaGranularity_) {
    throw VulkanProcNotFound("vkGetRenderAreaGranularity");
  }
  vkCreateCommandPool_ = reinterpret_cast<PFN_vkCreateCommandPool>(this->vkGetDeviceProcAddr("vkCreateCommandPool"));
  if (!vkCreateCommandPool_) {
    throw VulkanProcNotFound("vkCreateCommandPool");
  }
  vkDestroyCommandPool_ = reinterpret_cast<PFN_vkDestroyCommandPool>(this->vkGetDeviceProcAddr("vkDestroyCommandPool"));
  if (!vkDestroyCommandPool_) {
    throw VulkanProcNotFound("vkDestroyCommandPool");
  }
  vkResetCommandPool_ = reinterpret_cast<PFN_vkResetCommandPool>(this->vkGetDeviceProcAddr("vkResetCommandPool"));
  if (!vkResetCommandPool_) {
    throw VulkanProcNotFound("vkResetCommandPool");
  }
  vkAllocateCommandBuffers_ = reinterpret_cast<PFN_vkAllocateCommandBuffers>(this->vkGetDeviceProcAddr("vkAllocateCommandBuffers"));
  if (!vkAllocateCommandBuffers_) {
    throw VulkanProcNotFound("vkAllocateCommandBuffers");
  }
  vkFreeCommandBuffers_ = reinterpret_cast<PFN_vkFreeCommandBuffers>(this->vkGetDeviceProcAddr("vkFreeCommandBuffers"));
  if (!vkFreeCommandBuffers_) {
    throw VulkanProcNotFound("vkFreeCommandBuffers");
  }
  vkCreateSwapchainKHR_ = reinterpret_cast<PFN_vkCreateSwapchainKHR>(this->vkGetDeviceProcAddr("vkCreateSwapchainKHR"));
  vkDestroySwapchainKHR_ = reinterpret_cast<PFN_vkDestroySwapchainKHR>(this->vkGetDeviceProcAddr("vkDestroySwapchainKHR"));
  vkGetSwapchainImagesKHR_ = reinterpret_cast<PFN_vkGetSwapchainImagesKHR>(this->vkGetDeviceProcAddr("vkGetSwapchainImagesKHR"));
  vkAcquireNextImageKHR_ = reinterpret_cast<PFN_vkAcquireNextImageKHR>(this->vkGetDeviceProcAddr("vkAcquireNextImageKHR"));
  vkCreateSharedSwapchainsKHR_ = reinterpret_cast<PFN_vkCreateSharedSwapchainsKHR>(this->vkGetDeviceProcAddr("vkCreateSharedSwapchainsKHR"));
  vkDebugMarkerSetObjectTagEXT_ = reinterpret_cast<PFN_vkDebugMarkerSetObjectTagEXT>(this->vkGetDeviceProcAddr("vkDebugMarkerSetObjectTagEXT"));
  vkDebugMarkerSetObjectNameEXT_ = reinterpret_cast<PFN_vkDebugMarkerSetObjectNameEXT>(this->vkGetDeviceProcAddr("vkDebugMarkerSetObjectNameEXT"));
  vkGetShaderInfoAMD_ = reinterpret_cast<PFN_vkGetShaderInfoAMD>(this->vkGetDeviceProcAddr("vkGetShaderInfoAMD"));
#if defined(VK_USE_PLATFORM_WIN32_KHR)
  vkGetMemoryWin32HandleNV_ = reinterpret_cast<PFN_vkGetMemoryWin32HandleNV>(this->vkGetDeviceProcAddr("vkGetMemoryWin32HandleNV"));
#endif
  vkGetDeviceGroupPeerMemoryFeaturesKHX_ = reinterpret_cast<PFN_vkGetDeviceGroupPeerMemoryFeaturesKHX>(this->vkGetDeviceProcAddr("vkGetDeviceGroupPeerMemoryFeaturesKHX"));
  vkTrimCommandPoolKHR_ = reinterpret_cast<PFN_vkTrimCommandPoolKHR>(this->vkGetDeviceProcAddr("vkTrimCommandPoolKHR"));
#if defined(VK_USE_PLATFORM_WIN32_KHR)
  vkGetMemoryWin32HandleKHR_ = reinterpret_cast<PFN_vkGetMemoryWin32HandleKHR>(this->vkGetDeviceProcAddr("vkGetMemoryWin32HandleKHR"));
#endif
#if defined(VK_USE_PLATFORM_WIN32_KHR)
  vkGetMemoryWin32HandlePropertiesKHR_ = reinterpret_cast<PFN_vkGetMemoryWin32HandlePropertiesKHR>(this->vkGetDeviceProcAddr("vkGetMemoryWin32HandlePropertiesKHR"));
#endif
  vkGetMemoryFdKHR_ = reinterpret_cast<PFN_vkGetMemoryFdKHR>(this->vkGetDeviceProcAddr("vkGetMemoryFdKHR"));
  vkGetMemoryFdPropertiesKHR_ = reinterpret_cast<PFN_vkGetMemoryFdPropertiesKHR>(this->vkGetDeviceProcAddr("vkGetMemoryFdPropertiesKHR"));
#if defined(VK_USE_PLATFORM_WIN32_KHR)
  vkImportSemaphoreWin32HandleKHR_ = reinterpret_cast<PFN_vkImportSemaphoreWin32HandleKHR>(this->vkGetDeviceProcAddr("vkImportSemaphoreWin32HandleKHR"));
#endif
#if defined(VK_USE_PLATFORM_WIN32_KHR)
  vkGetSemaphoreWin32HandleKHR_ = reinterpret_cast<PFN_vkGetSemaphoreWin32HandleKHR>(this->vkGetDeviceProcAddr("vkGetSemaphoreWin32HandleKHR"));
#endif
  vkImportSemaphoreFdKHR_ = reinterpret_cast<PFN_vkImportSemaphoreFdKHR>(this->vkGetDeviceProcAddr("vkImportSemaphoreFdKHR"));
  vkGetSemaphoreFdKHR_ = reinterpret_cast<PFN_vkGetSemaphoreFdKHR>(this->vkGetDeviceProcAddr("vkGetSemaphoreFdKHR"));
  vkCreateDescriptorUpdateTemplateKHR_ = reinterpret_cast<PFN_vkCreateDescriptorUpdateTemplateKHR>(this->vkGetDeviceProcAddr("vkCreateDescriptorUpdateTemplateKHR"));
  vkDestroyDescriptorUpdateTemplateKHR_ = reinterpret_cast<PFN_vkDestroyDescriptorUpdateTemplateKHR>(this->vkGetDeviceProcAddr("vkDestroyDescriptorUpdateTemplateKHR"));
  vkUpdateDescriptorSetWithTemplateKHR_ = reinterpret_cast<PFN_vkUpdateDescriptorSetWithTemplateKHR>(this->vkGetDeviceProcAddr("vkUpdateDescriptorSetWithTemplateKHR"));
  vkCreateIndirectCommandsLayoutNVX_ = reinterpret_cast<PFN_vkCreateIndirectCommandsLayoutNVX>(this->vkGetDeviceProcAddr("vkCreateIndirectCommandsLayoutNVX"));
  vkDestroyIndirectCommandsLayoutNVX_ = reinterpret_cast<PFN_vkDestroyIndirectCommandsLayoutNVX>(this->vkGetDeviceProcAddr("vkDestroyIndirectCommandsLayoutNVX"));
  vkCreateObjectTableNVX_ = reinterpret_cast<PFN_vkCreateObjectTableNVX>(this->vkGetDeviceProcAddr("vkCreateObjectTableNVX"));
  vkDestroyObjectTableNVX_ = reinterpret_cast<PFN_vkDestroyObjectTableNVX>(this->vkGetDeviceProcAddr("vkDestroyObjectTableNVX"));
  vkRegisterObjectsNVX_ = reinterpret_cast<PFN_vkRegisterObjectsNVX>(this->vkGetDeviceProcAddr("vkRegisterObjectsNVX"));
  vkUnregisterObjectsNVX_ = reinterpret_cast<PFN_vkUnregisterObjectsNVX>(this->vkGetDeviceProcAddr("vkUnregisterObjectsNVX"));
  vkDisplayPowerControlEXT_ = reinterpret_cast<PFN_vkDisplayPowerControlEXT>(this->vkGetDeviceProcAddr("vkDisplayPowerControlEXT"));
  vkRegisterDeviceEventEXT_ = reinterpret_cast<PFN_vkRegisterDeviceEventEXT>(this->vkGetDeviceProcAddr("vkRegisterDeviceEventEXT"));
  vkRegisterDisplayEventEXT_ = reinterpret_cast<PFN_vkRegisterDisplayEventEXT>(this->vkGetDeviceProcAddr("vkRegisterDisplayEventEXT"));
  vkGetSwapchainCounterEXT_ = reinterpret_cast<PFN_vkGetSwapchainCounterEXT>(this->vkGetDeviceProcAddr("vkGetSwapchainCounterEXT"));
  vkGetRefreshCycleDurationGOOGLE_ = reinterpret_cast<PFN_vkGetRefreshCycleDurationGOOGLE>(this->vkGetDeviceProcAddr("vkGetRefreshCycleDurationGOOGLE"));
  vkGetPastPresentationTimingGOOGLE_ = reinterpret_cast<PFN_vkGetPastPresentationTimingGOOGLE>(this->vkGetDeviceProcAddr("vkGetPastPresentationTimingGOOGLE"));
  vkSetHdrMetadataEXT_ = reinterpret_cast<PFN_vkSetHdrMetadataEXT>(this->vkGetDeviceProcAddr("vkSetHdrMetadataEXT"));
  vkGetSwapchainStatusKHR_ = reinterpret_cast<PFN_vkGetSwapchainStatusKHR>(this->vkGetDeviceProcAddr("vkGetSwapchainStatusKHR"));
#if defined(VK_USE_PLATFORM_WIN32_KHR)
  vkImportFenceWin32HandleKHR_ = reinterpret_cast<PFN_vkImportFenceWin32HandleKHR>(this->vkGetDeviceProcAddr("vkImportFenceWin32HandleKHR"));
#endif
#if defined(VK_USE_PLATFORM_WIN32_KHR)
  vkGetFenceWin32HandleKHR_ = reinterpret_cast<PFN_vkGetFenceWin32HandleKHR>(this->vkGetDeviceProcAddr("vkGetFenceWin32HandleKHR"));
#endif
  vkImportFenceFdKHR_ = reinterpret_cast<PFN_vkImportFenceFdKHR>(this->vkGetDeviceProcAddr("vkImportFenceFdKHR"));
  vkGetFenceFdKHR_ = reinterpret_cast<PFN_vkGetFenceFdKHR>(this->vkGetDeviceProcAddr("vkGetFenceFdKHR"));
  vkGetImageMemoryRequirements2KHR_ = reinterpret_cast<PFN_vkGetImageMemoryRequirements2KHR>(this->vkGetDeviceProcAddr("vkGetImageMemoryRequirements2KHR"));
  vkGetBufferMemoryRequirements2KHR_ = reinterpret_cast<PFN_vkGetBufferMemoryRequirements2KHR>(this->vkGetDeviceProcAddr("vkGetBufferMemoryRequirements2KHR"));
  vkGetImageSparseMemoryRequirements2KHR_ = reinterpret_cast<PFN_vkGetImageSparseMemoryRequirements2KHR>(this->vkGetDeviceProcAddr("vkGetImageSparseMemoryRequirements2KHR"));
  vkCreateSamplerYcbcrConversionKHR_ = reinterpret_cast<PFN_vkCreateSamplerYcbcrConversionKHR>(this->vkGetDeviceProcAddr("vkCreateSamplerYcbcrConversionKHR"));
  vkDestroySamplerYcbcrConversionKHR_ = reinterpret_cast<PFN_vkDestroySamplerYcbcrConversionKHR>(this->vkGetDeviceProcAddr("vkDestroySamplerYcbcrConversionKHR"));
  vkBindBufferMemory2KHR_ = reinterpret_cast<PFN_vkBindBufferMemory2KHR>(this->vkGetDeviceProcAddr("vkBindBufferMemory2KHR"));
  vkBindImageMemory2KHR_ = reinterpret_cast<PFN_vkBindImageMemory2KHR>(this->vkGetDeviceProcAddr("vkBindImageMemory2KHR"));
  vkCreateValidationCacheEXT_ = reinterpret_cast<PFN_vkCreateValidationCacheEXT>(this->vkGetDeviceProcAddr("vkCreateValidationCacheEXT"));
  vkDestroyValidationCacheEXT_ = reinterpret_cast<PFN_vkDestroyValidationCacheEXT>(this->vkGetDeviceProcAddr("vkDestroyValidationCacheEXT"));
  vkMergeValidationCachesEXT_ = reinterpret_cast<PFN_vkMergeValidationCachesEXT>(this->vkGetDeviceProcAddr("vkMergeValidationCachesEXT"));
  vkGetValidationCacheDataEXT_ = reinterpret_cast<PFN_vkGetValidationCacheDataEXT>(this->vkGetDeviceProcAddr("vkGetValidationCacheDataEXT"));
}

/*
 * ------------------------------------------------------
 * QueueFunctions
 * ------------------------------------------------------
*/

VkResult QueueFunctions::vkQueueSubmit(uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence) const {
  return this->vkQueueSubmit_(queue_, submitCount, pSubmits, fence);
}

VkResult QueueFunctions::vkQueueWaitIdle() const {
  return this->vkQueueWaitIdle_(queue_);
}

VkResult QueueFunctions::vkQueueBindSparse(uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo, VkFence fence) const {
  return this->vkQueueBindSparse_(queue_, bindInfoCount, pBindInfo, fence);
}

VkResult QueueFunctions::vkQueuePresentKHR(const VkPresentInfoKHR* pPresentInfo) const {
  return this->vkQueuePresentKHR_(queue_, pPresentInfo);
}

QueueFunctions::QueueFunctions(VkQueue queue, DeviceFunctions* device) {
  queue_ = queue;
  vkQueueSubmit_ = reinterpret_cast<PFN_vkQueueSubmit>(device->vkGetDeviceProcAddr("vkQueueSubmit"));
  if (!vkQueueSubmit_) {
    throw VulkanProcNotFound("vkQueueSubmit");
  }
  vkQueueWaitIdle_ = reinterpret_cast<PFN_vkQueueWaitIdle>(device->vkGetDeviceProcAddr("vkQueueWaitIdle"));
  if (!vkQueueWaitIdle_) {
    throw VulkanProcNotFound("vkQueueWaitIdle");
  }
  vkQueueBindSparse_ = reinterpret_cast<PFN_vkQueueBindSparse>(device->vkGetDeviceProcAddr("vkQueueBindSparse"));
  if (!vkQueueBindSparse_) {
    throw VulkanProcNotFound("vkQueueBindSparse");
  }
  vkQueuePresentKHR_ = reinterpret_cast<PFN_vkQueuePresentKHR>(device->vkGetDeviceProcAddr("vkQueuePresentKHR"));
}

/*
 * ------------------------------------------------------
 * CommandBufferFunctions
 * ------------------------------------------------------
*/

VkResult CommandBufferFunctions::vkBeginCommandBuffer(const VkCommandBufferBeginInfo* pBeginInfo) const {
  return this->vkBeginCommandBuffer_(command_buffer_, pBeginInfo);
}

VkResult CommandBufferFunctions::vkEndCommandBuffer() const {
  return this->vkEndCommandBuffer_(command_buffer_);
}

VkResult CommandBufferFunctions::vkResetCommandBuffer(VkCommandBufferResetFlags flags) const {
  return this->vkResetCommandBuffer_(command_buffer_, flags);
}

void CommandBufferFunctions::vkCmdBindPipeline(VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline) const {
  return this->vkCmdBindPipeline_(command_buffer_, pipelineBindPoint, pipeline);
}

void CommandBufferFunctions::vkCmdSetViewport(uint32_t firstViewport, uint32_t viewportCount, const VkViewport* pViewports) const {
  return this->vkCmdSetViewport_(command_buffer_, firstViewport, viewportCount, pViewports);
}

void CommandBufferFunctions::vkCmdSetScissor(uint32_t firstScissor, uint32_t scissorCount, const VkRect2D* pScissors) const {
  return this->vkCmdSetScissor_(command_buffer_, firstScissor, scissorCount, pScissors);
}

void CommandBufferFunctions::vkCmdSetLineWidth(float lineWidth) const {
  return this->vkCmdSetLineWidth_(command_buffer_, lineWidth);
}

void CommandBufferFunctions::vkCmdSetDepthBias(float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor) const {
  return this->vkCmdSetDepthBias_(command_buffer_, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
}

void CommandBufferFunctions::vkCmdSetBlendConstants(const float blendConstants[4]) const {
  return this->vkCmdSetBlendConstants_(command_buffer_, blendConstants);
}

void CommandBufferFunctions::vkCmdSetDepthBounds(float minDepthBounds, float maxDepthBounds) const {
  return this->vkCmdSetDepthBounds_(command_buffer_, minDepthBounds, maxDepthBounds);
}

void CommandBufferFunctions::vkCmdSetStencilCompareMask(VkStencilFaceFlags faceMask, uint32_t compareMask) const {
  return this->vkCmdSetStencilCompareMask_(command_buffer_, faceMask, compareMask);
}

void CommandBufferFunctions::vkCmdSetStencilWriteMask(VkStencilFaceFlags faceMask, uint32_t writeMask) const {
  return this->vkCmdSetStencilWriteMask_(command_buffer_, faceMask, writeMask);
}

void CommandBufferFunctions::vkCmdSetStencilReference(VkStencilFaceFlags faceMask, uint32_t reference) const {
  return this->vkCmdSetStencilReference_(command_buffer_, faceMask, reference);
}

void CommandBufferFunctions::vkCmdBindDescriptorSets(VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets) const {
  return this->vkCmdBindDescriptorSets_(command_buffer_, pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
}

void CommandBufferFunctions::vkCmdBindIndexBuffer(VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType) const {
  return this->vkCmdBindIndexBuffer_(command_buffer_, buffer, offset, indexType);
}

void CommandBufferFunctions::vkCmdBindVertexBuffers(uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets) const {
  return this->vkCmdBindVertexBuffers_(command_buffer_, firstBinding, bindingCount, pBuffers, pOffsets);
}

void CommandBufferFunctions::vkCmdDraw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) const {
  return this->vkCmdDraw_(command_buffer_, vertexCount, instanceCount, firstVertex, firstInstance);
}

void CommandBufferFunctions::vkCmdDrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) const {
  return this->vkCmdDrawIndexed_(command_buffer_, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void CommandBufferFunctions::vkCmdDrawIndirect(VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride) const {
  return this->vkCmdDrawIndirect_(command_buffer_, buffer, offset, drawCount, stride);
}

void CommandBufferFunctions::vkCmdDrawIndexedIndirect(VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride) const {
  return this->vkCmdDrawIndexedIndirect_(command_buffer_, buffer, offset, drawCount, stride);
}

void CommandBufferFunctions::vkCmdDispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) const {
  return this->vkCmdDispatch_(command_buffer_, groupCountX, groupCountY, groupCountZ);
}

void CommandBufferFunctions::vkCmdDispatchIndirect(VkBuffer buffer, VkDeviceSize offset) const {
  return this->vkCmdDispatchIndirect_(command_buffer_, buffer, offset);
}

void CommandBufferFunctions::vkCmdCopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferCopy* pRegions) const {
  return this->vkCmdCopyBuffer_(command_buffer_, srcBuffer, dstBuffer, regionCount, pRegions);
}

void CommandBufferFunctions::vkCmdCopyImage(VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy* pRegions) const {
  return this->vkCmdCopyImage_(command_buffer_, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
}

void CommandBufferFunctions::vkCmdBlitImage(VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit* pRegions, VkFilter filter) const {
  return this->vkCmdBlitImage_(command_buffer_, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter);
}

void CommandBufferFunctions::vkCmdCopyBufferToImage(VkBuffer srcBuffer, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy* pRegions) const {
  return this->vkCmdCopyBufferToImage_(command_buffer_, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
}

void CommandBufferFunctions::vkCmdCopyImageToBuffer(VkImage srcImage, VkImageLayout srcImageLayout, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions) const {
  return this->vkCmdCopyImageToBuffer_(command_buffer_, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);
}

void CommandBufferFunctions::vkCmdUpdateBuffer(VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize dataSize, const void* pData) const {
  return this->vkCmdUpdateBuffer_(command_buffer_, dstBuffer, dstOffset, dataSize, pData);
}

void CommandBufferFunctions::vkCmdFillBuffer(VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size, uint32_t data) const {
  return this->vkCmdFillBuffer_(command_buffer_, dstBuffer, dstOffset, size, data);
}

void CommandBufferFunctions::vkCmdClearColorImage(VkImage image, VkImageLayout imageLayout, const VkClearColorValue* pColor, uint32_t rangeCount, const VkImageSubresourceRange* pRanges) const {
  return this->vkCmdClearColorImage_(command_buffer_, image, imageLayout, pColor, rangeCount, pRanges);
}

void CommandBufferFunctions::vkCmdClearDepthStencilImage(VkImage image, VkImageLayout imageLayout, const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount, const VkImageSubresourceRange* pRanges) const {
  return this->vkCmdClearDepthStencilImage_(command_buffer_, image, imageLayout, pDepthStencil, rangeCount, pRanges);
}

void CommandBufferFunctions::vkCmdClearAttachments(uint32_t attachmentCount, const VkClearAttachment* pAttachments, uint32_t rectCount, const VkClearRect* pRects) const {
  return this->vkCmdClearAttachments_(command_buffer_, attachmentCount, pAttachments, rectCount, pRects);
}

void CommandBufferFunctions::vkCmdResolveImage(VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageResolve* pRegions) const {
  return this->vkCmdResolveImage_(command_buffer_, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
}

void CommandBufferFunctions::vkCmdSetEvent(VkEvent event, VkPipelineStageFlags stageMask) const {
  return this->vkCmdSetEvent_(command_buffer_, event, stageMask);
}

void CommandBufferFunctions::vkCmdResetEvent(VkEvent event, VkPipelineStageFlags stageMask) const {
  return this->vkCmdResetEvent_(command_buffer_, event, stageMask);
}

void CommandBufferFunctions::vkCmdWaitEvents(uint32_t eventCount, const VkEvent* pEvents, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers) const {
  return this->vkCmdWaitEvents_(command_buffer_, eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
}

void CommandBufferFunctions::vkCmdPipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers) const {
  return this->vkCmdPipelineBarrier_(command_buffer_, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
}

void CommandBufferFunctions::vkCmdBeginQuery(VkQueryPool queryPool, uint32_t query, VkQueryControlFlags flags) const {
  return this->vkCmdBeginQuery_(command_buffer_, queryPool, query, flags);
}

void CommandBufferFunctions::vkCmdEndQuery(VkQueryPool queryPool, uint32_t query) const {
  return this->vkCmdEndQuery_(command_buffer_, queryPool, query);
}

void CommandBufferFunctions::vkCmdResetQueryPool(VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount) const {
  return this->vkCmdResetQueryPool_(command_buffer_, queryPool, firstQuery, queryCount);
}

void CommandBufferFunctions::vkCmdWriteTimestamp(VkPipelineStageFlagBits pipelineStage, VkQueryPool queryPool, uint32_t query) const {
  return this->vkCmdWriteTimestamp_(command_buffer_, pipelineStage, queryPool, query);
}

void CommandBufferFunctions::vkCmdCopyQueryPoolResults(VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride, VkQueryResultFlags flags) const {
  return this->vkCmdCopyQueryPoolResults_(command_buffer_, queryPool, firstQuery, queryCount, dstBuffer, dstOffset, stride, flags);
}

void CommandBufferFunctions::vkCmdPushConstants(VkPipelineLayout layout, VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* pValues) const {
  return this->vkCmdPushConstants_(command_buffer_, layout, stageFlags, offset, size, pValues);
}

void CommandBufferFunctions::vkCmdBeginRenderPass(const VkRenderPassBeginInfo* pRenderPassBegin, VkSubpassContents contents) const {
  return this->vkCmdBeginRenderPass_(command_buffer_, pRenderPassBegin, contents);
}

void CommandBufferFunctions::vkCmdNextSubpass(VkSubpassContents contents) const {
  return this->vkCmdNextSubpass_(command_buffer_, contents);
}

void CommandBufferFunctions::vkCmdEndRenderPass() const {
  return this->vkCmdEndRenderPass_(command_buffer_);
}

void CommandBufferFunctions::vkCmdExecuteCommands(uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers) const {
  return this->vkCmdExecuteCommands_(command_buffer_, commandBufferCount, pCommandBuffers);
}

void CommandBufferFunctions::vkCmdDebugMarkerBeginEXT(const VkDebugMarkerMarkerInfoEXT* pMarkerInfo) const {
  return this->vkCmdDebugMarkerBeginEXT_(command_buffer_, pMarkerInfo);
}

void CommandBufferFunctions::vkCmdDebugMarkerEndEXT() const {
  return this->vkCmdDebugMarkerEndEXT_(command_buffer_);
}

void CommandBufferFunctions::vkCmdDebugMarkerInsertEXT(const VkDebugMarkerMarkerInfoEXT* pMarkerInfo) const {
  return this->vkCmdDebugMarkerInsertEXT_(command_buffer_, pMarkerInfo);
}

void CommandBufferFunctions::vkCmdDrawIndirectCountAMD(VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) const {
  return this->vkCmdDrawIndirectCountAMD_(command_buffer_, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}

void CommandBufferFunctions::vkCmdDrawIndexedIndirectCountAMD(VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) const {
  return this->vkCmdDrawIndexedIndirectCountAMD_(command_buffer_, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}

void CommandBufferFunctions::vkCmdSetDeviceMaskKHX(uint32_t deviceMask) const {
  return this->vkCmdSetDeviceMaskKHX_(command_buffer_, deviceMask);
}

void CommandBufferFunctions::vkCmdDispatchBaseKHX(uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) const {
  return this->vkCmdDispatchBaseKHX_(command_buffer_, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
}

void CommandBufferFunctions::vkCmdPushDescriptorSetKHR(VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount, const VkWriteDescriptorSet* pDescriptorWrites) const {
  return this->vkCmdPushDescriptorSetKHR_(command_buffer_, pipelineBindPoint, layout, set, descriptorWriteCount, pDescriptorWrites);
}

void CommandBufferFunctions::vkCmdPushDescriptorSetWithTemplateKHR(VkDescriptorUpdateTemplateKHR descriptorUpdateTemplate, VkPipelineLayout layout, uint32_t set, const void* pData) const {
  return this->vkCmdPushDescriptorSetWithTemplateKHR_(command_buffer_, descriptorUpdateTemplate, layout, set, pData);
}

void CommandBufferFunctions::vkCmdProcessCommandsNVX(const VkCmdProcessCommandsInfoNVX* pProcessCommandsInfo) const {
  return this->vkCmdProcessCommandsNVX_(command_buffer_, pProcessCommandsInfo);
}

void CommandBufferFunctions::vkCmdReserveSpaceForCommandsNVX(const VkCmdReserveSpaceForCommandsInfoNVX* pReserveSpaceInfo) const {
  return this->vkCmdReserveSpaceForCommandsNVX_(command_buffer_, pReserveSpaceInfo);
}

void CommandBufferFunctions::vkCmdSetViewportWScalingNV(uint32_t firstViewport, uint32_t viewportCount, const VkViewportWScalingNV* pViewportWScalings) const {
  return this->vkCmdSetViewportWScalingNV_(command_buffer_, firstViewport, viewportCount, pViewportWScalings);
}

void CommandBufferFunctions::vkCmdSetDiscardRectangleEXT(uint32_t firstDiscardRectangle, uint32_t discardRectangleCount, const VkRect2D* pDiscardRectangles) const {
  return this->vkCmdSetDiscardRectangleEXT_(command_buffer_, firstDiscardRectangle, discardRectangleCount, pDiscardRectangles);
}

void CommandBufferFunctions::vkCmdSetSampleLocationsEXT(const VkSampleLocationsInfoEXT* pSampleLocationsInfo) const {
  return this->vkCmdSetSampleLocationsEXT_(command_buffer_, pSampleLocationsInfo);
}

CommandBufferFunctions::CommandBufferFunctions(VkCommandBuffer command_buffer, DeviceFunctions* device) {
  command_buffer_ = command_buffer;
  vkBeginCommandBuffer_ = reinterpret_cast<PFN_vkBeginCommandBuffer>(device->vkGetDeviceProcAddr("vkBeginCommandBuffer"));
  if (!vkBeginCommandBuffer_) {
    throw VulkanProcNotFound("vkBeginCommandBuffer");
  }
  vkEndCommandBuffer_ = reinterpret_cast<PFN_vkEndCommandBuffer>(device->vkGetDeviceProcAddr("vkEndCommandBuffer"));
  if (!vkEndCommandBuffer_) {
    throw VulkanProcNotFound("vkEndCommandBuffer");
  }
  vkResetCommandBuffer_ = reinterpret_cast<PFN_vkResetCommandBuffer>(device->vkGetDeviceProcAddr("vkResetCommandBuffer"));
  if (!vkResetCommandBuffer_) {
    throw VulkanProcNotFound("vkResetCommandBuffer");
  }
  vkCmdBindPipeline_ = reinterpret_cast<PFN_vkCmdBindPipeline>(device->vkGetDeviceProcAddr("vkCmdBindPipeline"));
  if (!vkCmdBindPipeline_) {
    throw VulkanProcNotFound("vkCmdBindPipeline");
  }
  vkCmdSetViewport_ = reinterpret_cast<PFN_vkCmdSetViewport>(device->vkGetDeviceProcAddr("vkCmdSetViewport"));
  if (!vkCmdSetViewport_) {
    throw VulkanProcNotFound("vkCmdSetViewport");
  }
  vkCmdSetScissor_ = reinterpret_cast<PFN_vkCmdSetScissor>(device->vkGetDeviceProcAddr("vkCmdSetScissor"));
  if (!vkCmdSetScissor_) {
    throw VulkanProcNotFound("vkCmdSetScissor");
  }
  vkCmdSetLineWidth_ = reinterpret_cast<PFN_vkCmdSetLineWidth>(device->vkGetDeviceProcAddr("vkCmdSetLineWidth"));
  if (!vkCmdSetLineWidth_) {
    throw VulkanProcNotFound("vkCmdSetLineWidth");
  }
  vkCmdSetDepthBias_ = reinterpret_cast<PFN_vkCmdSetDepthBias>(device->vkGetDeviceProcAddr("vkCmdSetDepthBias"));
  if (!vkCmdSetDepthBias_) {
    throw VulkanProcNotFound("vkCmdSetDepthBias");
  }
  vkCmdSetBlendConstants_ = reinterpret_cast<PFN_vkCmdSetBlendConstants>(device->vkGetDeviceProcAddr("vkCmdSetBlendConstants"));
  if (!vkCmdSetBlendConstants_) {
    throw VulkanProcNotFound("vkCmdSetBlendConstants");
  }
  vkCmdSetDepthBounds_ = reinterpret_cast<PFN_vkCmdSetDepthBounds>(device->vkGetDeviceProcAddr("vkCmdSetDepthBounds"));
  if (!vkCmdSetDepthBounds_) {
    throw VulkanProcNotFound("vkCmdSetDepthBounds");
  }
  vkCmdSetStencilCompareMask_ = reinterpret_cast<PFN_vkCmdSetStencilCompareMask>(device->vkGetDeviceProcAddr("vkCmdSetStencilCompareMask"));
  if (!vkCmdSetStencilCompareMask_) {
    throw VulkanProcNotFound("vkCmdSetStencilCompareMask");
  }
  vkCmdSetStencilWriteMask_ = reinterpret_cast<PFN_vkCmdSetStencilWriteMask>(device->vkGetDeviceProcAddr("vkCmdSetStencilWriteMask"));
  if (!vkCmdSetStencilWriteMask_) {
    throw VulkanProcNotFound("vkCmdSetStencilWriteMask");
  }
  vkCmdSetStencilReference_ = reinterpret_cast<PFN_vkCmdSetStencilReference>(device->vkGetDeviceProcAddr("vkCmdSetStencilReference"));
  if (!vkCmdSetStencilReference_) {
    throw VulkanProcNotFound("vkCmdSetStencilReference");
  }
  vkCmdBindDescriptorSets_ = reinterpret_cast<PFN_vkCmdBindDescriptorSets>(device->vkGetDeviceProcAddr("vkCmdBindDescriptorSets"));
  if (!vkCmdBindDescriptorSets_) {
    throw VulkanProcNotFound("vkCmdBindDescriptorSets");
  }
  vkCmdBindIndexBuffer_ = reinterpret_cast<PFN_vkCmdBindIndexBuffer>(device->vkGetDeviceProcAddr("vkCmdBindIndexBuffer"));
  if (!vkCmdBindIndexBuffer_) {
    throw VulkanProcNotFound("vkCmdBindIndexBuffer");
  }
  vkCmdBindVertexBuffers_ = reinterpret_cast<PFN_vkCmdBindVertexBuffers>(device->vkGetDeviceProcAddr("vkCmdBindVertexBuffers"));
  if (!vkCmdBindVertexBuffers_) {
    throw VulkanProcNotFound("vkCmdBindVertexBuffers");
  }
  vkCmdDraw_ = reinterpret_cast<PFN_vkCmdDraw>(device->vkGetDeviceProcAddr("vkCmdDraw"));
  if (!vkCmdDraw_) {
    throw VulkanProcNotFound("vkCmdDraw");
  }
  vkCmdDrawIndexed_ = reinterpret_cast<PFN_vkCmdDrawIndexed>(device->vkGetDeviceProcAddr("vkCmdDrawIndexed"));
  if (!vkCmdDrawIndexed_) {
    throw VulkanProcNotFound("vkCmdDrawIndexed");
  }
  vkCmdDrawIndirect_ = reinterpret_cast<PFN_vkCmdDrawIndirect>(device->vkGetDeviceProcAddr("vkCmdDrawIndirect"));
  if (!vkCmdDrawIndirect_) {
    throw VulkanProcNotFound("vkCmdDrawIndirect");
  }
  vkCmdDrawIndexedIndirect_ = reinterpret_cast<PFN_vkCmdDrawIndexedIndirect>(device->vkGetDeviceProcAddr("vkCmdDrawIndexedIndirect"));
  if (!vkCmdDrawIndexedIndirect_) {
    throw VulkanProcNotFound("vkCmdDrawIndexedIndirect");
  }
  vkCmdDispatch_ = reinterpret_cast<PFN_vkCmdDispatch>(device->vkGetDeviceProcAddr("vkCmdDispatch"));
  if (!vkCmdDispatch_) {
    throw VulkanProcNotFound("vkCmdDispatch");
  }
  vkCmdDispatchIndirect_ = reinterpret_cast<PFN_vkCmdDispatchIndirect>(device->vkGetDeviceProcAddr("vkCmdDispatchIndirect"));
  if (!vkCmdDispatchIndirect_) {
    throw VulkanProcNotFound("vkCmdDispatchIndirect");
  }
  vkCmdCopyBuffer_ = reinterpret_cast<PFN_vkCmdCopyBuffer>(device->vkGetDeviceProcAddr("vkCmdCopyBuffer"));
  if (!vkCmdCopyBuffer_) {
    throw VulkanProcNotFound("vkCmdCopyBuffer");
  }
  vkCmdCopyImage_ = reinterpret_cast<PFN_vkCmdCopyImage>(device->vkGetDeviceProcAddr("vkCmdCopyImage"));
  if (!vkCmdCopyImage_) {
    throw VulkanProcNotFound("vkCmdCopyImage");
  }
  vkCmdBlitImage_ = reinterpret_cast<PFN_vkCmdBlitImage>(device->vkGetDeviceProcAddr("vkCmdBlitImage"));
  if (!vkCmdBlitImage_) {
    throw VulkanProcNotFound("vkCmdBlitImage");
  }
  vkCmdCopyBufferToImage_ = reinterpret_cast<PFN_vkCmdCopyBufferToImage>(device->vkGetDeviceProcAddr("vkCmdCopyBufferToImage"));
  if (!vkCmdCopyBufferToImage_) {
    throw VulkanProcNotFound("vkCmdCopyBufferToImage");
  }
  vkCmdCopyImageToBuffer_ = reinterpret_cast<PFN_vkCmdCopyImageToBuffer>(device->vkGetDeviceProcAddr("vkCmdCopyImageToBuffer"));
  if (!vkCmdCopyImageToBuffer_) {
    throw VulkanProcNotFound("vkCmdCopyImageToBuffer");
  }
  vkCmdUpdateBuffer_ = reinterpret_cast<PFN_vkCmdUpdateBuffer>(device->vkGetDeviceProcAddr("vkCmdUpdateBuffer"));
  if (!vkCmdUpdateBuffer_) {
    throw VulkanProcNotFound("vkCmdUpdateBuffer");
  }
  vkCmdFillBuffer_ = reinterpret_cast<PFN_vkCmdFillBuffer>(device->vkGetDeviceProcAddr("vkCmdFillBuffer"));
  if (!vkCmdFillBuffer_) {
    throw VulkanProcNotFound("vkCmdFillBuffer");
  }
  vkCmdClearColorImage_ = reinterpret_cast<PFN_vkCmdClearColorImage>(device->vkGetDeviceProcAddr("vkCmdClearColorImage"));
  if (!vkCmdClearColorImage_) {
    throw VulkanProcNotFound("vkCmdClearColorImage");
  }
  vkCmdClearDepthStencilImage_ = reinterpret_cast<PFN_vkCmdClearDepthStencilImage>(device->vkGetDeviceProcAddr("vkCmdClearDepthStencilImage"));
  if (!vkCmdClearDepthStencilImage_) {
    throw VulkanProcNotFound("vkCmdClearDepthStencilImage");
  }
  vkCmdClearAttachments_ = reinterpret_cast<PFN_vkCmdClearAttachments>(device->vkGetDeviceProcAddr("vkCmdClearAttachments"));
  if (!vkCmdClearAttachments_) {
    throw VulkanProcNotFound("vkCmdClearAttachments");
  }
  vkCmdResolveImage_ = reinterpret_cast<PFN_vkCmdResolveImage>(device->vkGetDeviceProcAddr("vkCmdResolveImage"));
  if (!vkCmdResolveImage_) {
    throw VulkanProcNotFound("vkCmdResolveImage");
  }
  vkCmdSetEvent_ = reinterpret_cast<PFN_vkCmdSetEvent>(device->vkGetDeviceProcAddr("vkCmdSetEvent"));
  if (!vkCmdSetEvent_) {
    throw VulkanProcNotFound("vkCmdSetEvent");
  }
  vkCmdResetEvent_ = reinterpret_cast<PFN_vkCmdResetEvent>(device->vkGetDeviceProcAddr("vkCmdResetEvent"));
  if (!vkCmdResetEvent_) {
    throw VulkanProcNotFound("vkCmdResetEvent");
  }
  vkCmdWaitEvents_ = reinterpret_cast<PFN_vkCmdWaitEvents>(device->vkGetDeviceProcAddr("vkCmdWaitEvents"));
  if (!vkCmdWaitEvents_) {
    throw VulkanProcNotFound("vkCmdWaitEvents");
  }
  vkCmdPipelineBarrier_ = reinterpret_cast<PFN_vkCmdPipelineBarrier>(device->vkGetDeviceProcAddr("vkCmdPipelineBarrier"));
  if (!vkCmdPipelineBarrier_) {
    throw VulkanProcNotFound("vkCmdPipelineBarrier");
  }
  vkCmdBeginQuery_ = reinterpret_cast<PFN_vkCmdBeginQuery>(device->vkGetDeviceProcAddr("vkCmdBeginQuery"));
  if (!vkCmdBeginQuery_) {
    throw VulkanProcNotFound("vkCmdBeginQuery");
  }
  vkCmdEndQuery_ = reinterpret_cast<PFN_vkCmdEndQuery>(device->vkGetDeviceProcAddr("vkCmdEndQuery"));
  if (!vkCmdEndQuery_) {
    throw VulkanProcNotFound("vkCmdEndQuery");
  }
  vkCmdResetQueryPool_ = reinterpret_cast<PFN_vkCmdResetQueryPool>(device->vkGetDeviceProcAddr("vkCmdResetQueryPool"));
  if (!vkCmdResetQueryPool_) {
    throw VulkanProcNotFound("vkCmdResetQueryPool");
  }
  vkCmdWriteTimestamp_ = reinterpret_cast<PFN_vkCmdWriteTimestamp>(device->vkGetDeviceProcAddr("vkCmdWriteTimestamp"));
  if (!vkCmdWriteTimestamp_) {
    throw VulkanProcNotFound("vkCmdWriteTimestamp");
  }
  vkCmdCopyQueryPoolResults_ = reinterpret_cast<PFN_vkCmdCopyQueryPoolResults>(device->vkGetDeviceProcAddr("vkCmdCopyQueryPoolResults"));
  if (!vkCmdCopyQueryPoolResults_) {
    throw VulkanProcNotFound("vkCmdCopyQueryPoolResults");
  }
  vkCmdPushConstants_ = reinterpret_cast<PFN_vkCmdPushConstants>(device->vkGetDeviceProcAddr("vkCmdPushConstants"));
  if (!vkCmdPushConstants_) {
    throw VulkanProcNotFound("vkCmdPushConstants");
  }
  vkCmdBeginRenderPass_ = reinterpret_cast<PFN_vkCmdBeginRenderPass>(device->vkGetDeviceProcAddr("vkCmdBeginRenderPass"));
  if (!vkCmdBeginRenderPass_) {
    throw VulkanProcNotFound("vkCmdBeginRenderPass");
  }
  vkCmdNextSubpass_ = reinterpret_cast<PFN_vkCmdNextSubpass>(device->vkGetDeviceProcAddr("vkCmdNextSubpass"));
  if (!vkCmdNextSubpass_) {
    throw VulkanProcNotFound("vkCmdNextSubpass");
  }
  vkCmdEndRenderPass_ = reinterpret_cast<PFN_vkCmdEndRenderPass>(device->vkGetDeviceProcAddr("vkCmdEndRenderPass"));
  if (!vkCmdEndRenderPass_) {
    throw VulkanProcNotFound("vkCmdEndRenderPass");
  }
  vkCmdExecuteCommands_ = reinterpret_cast<PFN_vkCmdExecuteCommands>(device->vkGetDeviceProcAddr("vkCmdExecuteCommands"));
  if (!vkCmdExecuteCommands_) {
    throw VulkanProcNotFound("vkCmdExecuteCommands");
  }
  vkCmdDebugMarkerBeginEXT_ = reinterpret_cast<PFN_vkCmdDebugMarkerBeginEXT>(device->vkGetDeviceProcAddr("vkCmdDebugMarkerBeginEXT"));
  vkCmdDebugMarkerEndEXT_ = reinterpret_cast<PFN_vkCmdDebugMarkerEndEXT>(device->vkGetDeviceProcAddr("vkCmdDebugMarkerEndEXT"));
  vkCmdDebugMarkerInsertEXT_ = reinterpret_cast<PFN_vkCmdDebugMarkerInsertEXT>(device->vkGetDeviceProcAddr("vkCmdDebugMarkerInsertEXT"));
  vkCmdDrawIndirectCountAMD_ = reinterpret_cast<PFN_vkCmdDrawIndirectCountAMD>(device->vkGetDeviceProcAddr("vkCmdDrawIndirectCountAMD"));
  vkCmdDrawIndexedIndirectCountAMD_ = reinterpret_cast<PFN_vkCmdDrawIndexedIndirectCountAMD>(device->vkGetDeviceProcAddr("vkCmdDrawIndexedIndirectCountAMD"));
  vkCmdSetDeviceMaskKHX_ = reinterpret_cast<PFN_vkCmdSetDeviceMaskKHX>(device->vkGetDeviceProcAddr("vkCmdSetDeviceMaskKHX"));
  vkCmdDispatchBaseKHX_ = reinterpret_cast<PFN_vkCmdDispatchBaseKHX>(device->vkGetDeviceProcAddr("vkCmdDispatchBaseKHX"));
  vkCmdPushDescriptorSetKHR_ = reinterpret_cast<PFN_vkCmdPushDescriptorSetKHR>(device->vkGetDeviceProcAddr("vkCmdPushDescriptorSetKHR"));
  vkCmdPushDescriptorSetWithTemplateKHR_ = reinterpret_cast<PFN_vkCmdPushDescriptorSetWithTemplateKHR>(device->vkGetDeviceProcAddr("vkCmdPushDescriptorSetWithTemplateKHR"));
  vkCmdProcessCommandsNVX_ = reinterpret_cast<PFN_vkCmdProcessCommandsNVX>(device->vkGetDeviceProcAddr("vkCmdProcessCommandsNVX"));
  vkCmdReserveSpaceForCommandsNVX_ = reinterpret_cast<PFN_vkCmdReserveSpaceForCommandsNVX>(device->vkGetDeviceProcAddr("vkCmdReserveSpaceForCommandsNVX"));
  vkCmdSetViewportWScalingNV_ = reinterpret_cast<PFN_vkCmdSetViewportWScalingNV>(device->vkGetDeviceProcAddr("vkCmdSetViewportWScalingNV"));
  vkCmdSetDiscardRectangleEXT_ = reinterpret_cast<PFN_vkCmdSetDiscardRectangleEXT>(device->vkGetDeviceProcAddr("vkCmdSetDiscardRectangleEXT"));
  vkCmdSetSampleLocationsEXT_ = reinterpret_cast<PFN_vkCmdSetSampleLocationsEXT>(device->vkGetDeviceProcAddr("vkCmdSetSampleLocationsEXT"));
}

} // vkgen
