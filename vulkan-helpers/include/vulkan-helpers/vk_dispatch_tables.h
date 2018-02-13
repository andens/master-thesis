// Copyright (c) 2015-2017 The Khronos Group Inc.
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Dispatch tables for Vulkan 1.0.65, generated from the Khronos Vulkan API XML Registry.
// See https://github.com/andens/Vulkan-Hpp for generator details.

#ifndef VK_DISPATCH_TABLES_INCLUDE
#define VK_DISPATCH_TABLES_INCLUDE

#include "vulkan_include.inl"
#include <stdexcept>
#include <string>
#if defined(_WIN32)
#include <Windows.h>
#endif

namespace vkgen {

class VulkanProcNotFound: public std::exception {
public:
  VulkanProcNotFound(std::string const& proc) : proc_(proc) {}
  virtual const char* what() const throw() {
    return "Write better error message here";
  }

private:
  std::string proc_;
};

class GlobalFunctions {
#if defined(_WIN32)
  typedef HMODULE library_handle;
#elif defined(__linux__)
  typedef void* library_handle;
#else
#error "Unsupported OS"
#endif

public:
  GlobalFunctions(std::string const& vulkan_library);
  ~GlobalFunctions();
  PFN_vkVoidFunction vkGetInstanceProcAddr(VkInstance instance, const char* pName) const;
  VkResult vkCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance) const;
  VkResult vkEnumerateInstanceExtensionProperties(const char* pLayerName, uint32_t* pPropertyCount, VkExtensionProperties* pProperties) const;
  VkResult vkEnumerateInstanceLayerProperties(uint32_t* pPropertyCount, VkLayerProperties* pProperties) const;

private:
  GlobalFunctions(GlobalFunctions& other) = delete;
  void operator=(GlobalFunctions& rhs) = delete;

private:
  library_handle library_ = nullptr;
  PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr_ = nullptr;
  PFN_vkCreateInstance vkCreateInstance_ = nullptr;
  PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties_ = nullptr;
  PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties_ = nullptr;
};

class InstanceFunctions {
public:
  VkInstance instance() const { return instance_; }
  PFN_vkVoidFunction vkGetInstanceProcAddr(const char* pName) const;
  void vkDestroyInstance(const VkAllocationCallbacks* pAllocator) const;
  VkResult vkEnumeratePhysicalDevices(uint32_t* pPhysicalDeviceCount, VkPhysicalDevice* pPhysicalDevices) const;
  void vkDestroySurfaceKHR(VkSurfaceKHR surface, const VkAllocationCallbacks* pAllocator) const;
  VkResult vkCreateDisplayPlaneSurfaceKHR(const VkDisplaySurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const;
#if defined(VK_USE_PLATFORM_XLIB_KHR)
  VkResult vkCreateXlibSurfaceKHR(const VkXlibSurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const;
#endif
#if defined(VK_USE_PLATFORM_XCB_KHR)
  VkResult vkCreateXcbSurfaceKHR(const VkXcbSurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const;
#endif
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
  VkResult vkCreateWaylandSurfaceKHR(const VkWaylandSurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const;
#endif
#if defined(VK_USE_PLATFORM_MIR_KHR)
  VkResult vkCreateMirSurfaceKHR(const VkMirSurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const;
#endif
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
  VkResult vkCreateAndroidSurfaceKHR(const VkAndroidSurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const;
#endif
#if defined(VK_USE_PLATFORM_WIN32_KHR)
  VkResult vkCreateWin32SurfaceKHR(const VkWin32SurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const;
#endif
  VkResult vkCreateDebugReportCallbackEXT(const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback) const;
  void vkDestroyDebugReportCallbackEXT(VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator) const;
  void vkDebugReportMessageEXT(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage) const;
#if defined(VK_USE_PLATFORM_VI_NN)
  VkResult vkCreateViSurfaceNN(const VkViSurfaceCreateInfoNN* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const;
#endif
  VkResult vkEnumeratePhysicalDeviceGroupsKHX(uint32_t* pPhysicalDeviceGroupCount, VkPhysicalDeviceGroupPropertiesKHX* pPhysicalDeviceGroupProperties) const;
#if defined(VK_USE_PLATFORM_IOS_MVK)
  VkResult vkCreateIOSSurfaceMVK(const VkIOSSurfaceCreateInfoMVK* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const;
#endif
#if defined(VK_USE_PLATFORM_MACOS_MVK)
  VkResult vkCreateMacOSSurfaceMVK(const VkMacOSSurfaceCreateInfoMVK* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const;
#endif

protected:
  InstanceFunctions(VkInstance instance, GlobalFunctions* globals);

private:
  VkInstance instance_ = VK_NULL_HANDLE;
  PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr_ = nullptr;
  PFN_vkDestroyInstance vkDestroyInstance_ = nullptr;
  PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices_ = nullptr;
  PFN_vkDestroySurfaceKHR vkDestroySurfaceKHR_ = nullptr;
  PFN_vkCreateDisplayPlaneSurfaceKHR vkCreateDisplayPlaneSurfaceKHR_ = nullptr;
#if defined(VK_USE_PLATFORM_XLIB_KHR)
  PFN_vkCreateXlibSurfaceKHR vkCreateXlibSurfaceKHR_ = nullptr;
#endif
#if defined(VK_USE_PLATFORM_XCB_KHR)
  PFN_vkCreateXcbSurfaceKHR vkCreateXcbSurfaceKHR_ = nullptr;
#endif
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
  PFN_vkCreateWaylandSurfaceKHR vkCreateWaylandSurfaceKHR_ = nullptr;
#endif
#if defined(VK_USE_PLATFORM_MIR_KHR)
  PFN_vkCreateMirSurfaceKHR vkCreateMirSurfaceKHR_ = nullptr;
#endif
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
  PFN_vkCreateAndroidSurfaceKHR vkCreateAndroidSurfaceKHR_ = nullptr;
#endif
#if defined(VK_USE_PLATFORM_WIN32_KHR)
  PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR_ = nullptr;
#endif
  PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT_ = nullptr;
  PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT_ = nullptr;
  PFN_vkDebugReportMessageEXT vkDebugReportMessageEXT_ = nullptr;
#if defined(VK_USE_PLATFORM_VI_NN)
  PFN_vkCreateViSurfaceNN vkCreateViSurfaceNN_ = nullptr;
#endif
  PFN_vkEnumeratePhysicalDeviceGroupsKHX vkEnumeratePhysicalDeviceGroupsKHX_ = nullptr;
#if defined(VK_USE_PLATFORM_IOS_MVK)
  PFN_vkCreateIOSSurfaceMVK vkCreateIOSSurfaceMVK_ = nullptr;
#endif
#if defined(VK_USE_PLATFORM_MACOS_MVK)
  PFN_vkCreateMacOSSurfaceMVK vkCreateMacOSSurfaceMVK_ = nullptr;
#endif
};

class PhysicalDeviceFunctions {
public:
  VkPhysicalDevice physical_device() const { return physical_device_; }
  void vkGetPhysicalDeviceFeatures(VkPhysicalDeviceFeatures* pFeatures) const;
  void vkGetPhysicalDeviceFormatProperties(VkFormat format, VkFormatProperties* pFormatProperties) const;
  VkResult vkGetPhysicalDeviceImageFormatProperties(VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags, VkImageFormatProperties* pImageFormatProperties) const;
  void vkGetPhysicalDeviceProperties(VkPhysicalDeviceProperties* pProperties) const;
  void vkGetPhysicalDeviceQueueFamilyProperties(uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties* pQueueFamilyProperties) const;
  void vkGetPhysicalDeviceMemoryProperties(VkPhysicalDeviceMemoryProperties* pMemoryProperties) const;
  VkResult vkCreateDevice(const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice) const;
  VkResult vkEnumerateDeviceExtensionProperties(const char* pLayerName, uint32_t* pPropertyCount, VkExtensionProperties* pProperties) const;
  VkResult vkEnumerateDeviceLayerProperties(uint32_t* pPropertyCount, VkLayerProperties* pProperties) const;
  void vkGetPhysicalDeviceSparseImageFormatProperties(VkFormat format, VkImageType type, VkSampleCountFlagBits samples, VkImageUsageFlags usage, VkImageTiling tiling, uint32_t* pPropertyCount, VkSparseImageFormatProperties* pProperties) const;
  VkResult vkGetPhysicalDeviceSurfaceSupportKHR(uint32_t queueFamilyIndex, VkSurfaceKHR surface, VkBool32* pSupported) const;
  VkResult vkGetPhysicalDeviceSurfaceCapabilitiesKHR(VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR* pSurfaceCapabilities) const;
  VkResult vkGetPhysicalDeviceSurfaceFormatsKHR(VkSurfaceKHR surface, uint32_t* pSurfaceFormatCount, VkSurfaceFormatKHR* pSurfaceFormats) const;
  VkResult vkGetPhysicalDeviceSurfacePresentModesKHR(VkSurfaceKHR surface, uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes) const;
  VkResult vkGetPhysicalDeviceDisplayPropertiesKHR(uint32_t* pPropertyCount, VkDisplayPropertiesKHR* pProperties) const;
  VkResult vkGetPhysicalDeviceDisplayPlanePropertiesKHR(uint32_t* pPropertyCount, VkDisplayPlanePropertiesKHR* pProperties) const;
  VkResult vkGetDisplayPlaneSupportedDisplaysKHR(uint32_t planeIndex, uint32_t* pDisplayCount, VkDisplayKHR* pDisplays) const;
  VkResult vkGetDisplayModePropertiesKHR(VkDisplayKHR display, uint32_t* pPropertyCount, VkDisplayModePropertiesKHR* pProperties) const;
  VkResult vkCreateDisplayModeKHR(VkDisplayKHR display, const VkDisplayModeCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDisplayModeKHR* pMode) const;
  VkResult vkGetDisplayPlaneCapabilitiesKHR(VkDisplayModeKHR mode, uint32_t planeIndex, VkDisplayPlaneCapabilitiesKHR* pCapabilities) const;
#if defined(VK_USE_PLATFORM_XLIB_KHR)
  VkBool32 vkGetPhysicalDeviceXlibPresentationSupportKHR(uint32_t queueFamilyIndex, Display* dpy, VisualID visualID) const;
#endif
#if defined(VK_USE_PLATFORM_XCB_KHR)
  VkBool32 vkGetPhysicalDeviceXcbPresentationSupportKHR(uint32_t queueFamilyIndex, xcb_connection_t* connection, xcb_visualid_t visual_id) const;
#endif
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
  VkBool32 vkGetPhysicalDeviceWaylandPresentationSupportKHR(uint32_t queueFamilyIndex, wl_display* display) const;
#endif
#if defined(VK_USE_PLATFORM_MIR_KHR)
  VkBool32 vkGetPhysicalDeviceMirPresentationSupportKHR(uint32_t queueFamilyIndex, MirConnection* connection) const;
#endif
#if defined(VK_USE_PLATFORM_WIN32_KHR)
  VkBool32 vkGetPhysicalDeviceWin32PresentationSupportKHR(uint32_t queueFamilyIndex) const;
#endif
  VkResult vkGetPhysicalDeviceExternalImageFormatPropertiesNV(VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags, VkExternalMemoryHandleTypeFlagsNV externalHandleType, VkExternalImageFormatPropertiesNV* pExternalImageFormatProperties) const;
  void vkGetPhysicalDeviceFeatures2KHR(VkPhysicalDeviceFeatures2KHR* pFeatures) const;
  void vkGetPhysicalDeviceProperties2KHR(VkPhysicalDeviceProperties2KHR* pProperties) const;
  void vkGetPhysicalDeviceFormatProperties2KHR(VkFormat format, VkFormatProperties2KHR* pFormatProperties) const;
  VkResult vkGetPhysicalDeviceImageFormatProperties2KHR(const VkPhysicalDeviceImageFormatInfo2KHR* pImageFormatInfo, VkImageFormatProperties2KHR* pImageFormatProperties) const;
  void vkGetPhysicalDeviceQueueFamilyProperties2KHR(uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties2KHR* pQueueFamilyProperties) const;
  void vkGetPhysicalDeviceMemoryProperties2KHR(VkPhysicalDeviceMemoryProperties2KHR* pMemoryProperties) const;
  void vkGetPhysicalDeviceSparseImageFormatProperties2KHR(const VkPhysicalDeviceSparseImageFormatInfo2KHR* pFormatInfo, uint32_t* pPropertyCount, VkSparseImageFormatProperties2KHR* pProperties) const;
  void vkGetPhysicalDeviceExternalBufferPropertiesKHR(const VkPhysicalDeviceExternalBufferInfoKHR* pExternalBufferInfo, VkExternalBufferPropertiesKHR* pExternalBufferProperties) const;
  void vkGetPhysicalDeviceExternalSemaphorePropertiesKHR(const VkPhysicalDeviceExternalSemaphoreInfoKHR* pExternalSemaphoreInfo, VkExternalSemaphorePropertiesKHR* pExternalSemaphoreProperties) const;
  void vkGetPhysicalDeviceGeneratedCommandsPropertiesNVX(VkDeviceGeneratedCommandsFeaturesNVX* pFeatures, VkDeviceGeneratedCommandsLimitsNVX* pLimits) const;
  VkResult vkReleaseDisplayEXT(VkDisplayKHR display) const;
#if defined(VK_USE_PLATFORM_XLIB_XRANDR_EXT)
  VkResult vkAcquireXlibDisplayEXT(Display* dpy, VkDisplayKHR display) const;
#endif
#if defined(VK_USE_PLATFORM_XLIB_XRANDR_EXT)
  VkResult vkGetRandROutputDisplayEXT(Display* dpy, RROutput rrOutput, VkDisplayKHR* pDisplay) const;
#endif
  VkResult vkGetPhysicalDeviceSurfaceCapabilities2EXT(VkSurfaceKHR surface, VkSurfaceCapabilities2EXT* pSurfaceCapabilities) const;
  void vkGetPhysicalDeviceExternalFencePropertiesKHR(const VkPhysicalDeviceExternalFenceInfoKHR* pExternalFenceInfo, VkExternalFencePropertiesKHR* pExternalFenceProperties) const;
  VkResult vkGetPhysicalDeviceSurfaceCapabilities2KHR(const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo, VkSurfaceCapabilities2KHR* pSurfaceCapabilities) const;
  VkResult vkGetPhysicalDeviceSurfaceFormats2KHR(const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo, uint32_t* pSurfaceFormatCount, VkSurfaceFormat2KHR* pSurfaceFormats) const;
  void vkGetPhysicalDeviceMultisamplePropertiesEXT(VkSampleCountFlagBits samples, VkMultisamplePropertiesEXT* pMultisampleProperties) const;

protected:
  PhysicalDeviceFunctions(VkPhysicalDevice physical_device, InstanceFunctions* instance);

private:
  VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
  PFN_vkGetPhysicalDeviceFeatures vkGetPhysicalDeviceFeatures_ = nullptr;
  PFN_vkGetPhysicalDeviceFormatProperties vkGetPhysicalDeviceFormatProperties_ = nullptr;
  PFN_vkGetPhysicalDeviceImageFormatProperties vkGetPhysicalDeviceImageFormatProperties_ = nullptr;
  PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties_ = nullptr;
  PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties_ = nullptr;
  PFN_vkGetPhysicalDeviceMemoryProperties vkGetPhysicalDeviceMemoryProperties_ = nullptr;
  PFN_vkCreateDevice vkCreateDevice_ = nullptr;
  PFN_vkEnumerateDeviceExtensionProperties vkEnumerateDeviceExtensionProperties_ = nullptr;
  PFN_vkEnumerateDeviceLayerProperties vkEnumerateDeviceLayerProperties_ = nullptr;
  PFN_vkGetPhysicalDeviceSparseImageFormatProperties vkGetPhysicalDeviceSparseImageFormatProperties_ = nullptr;
  PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR_ = nullptr;
  PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR_ = nullptr;
  PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR_ = nullptr;
  PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR_ = nullptr;
  PFN_vkGetPhysicalDeviceDisplayPropertiesKHR vkGetPhysicalDeviceDisplayPropertiesKHR_ = nullptr;
  PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR vkGetPhysicalDeviceDisplayPlanePropertiesKHR_ = nullptr;
  PFN_vkGetDisplayPlaneSupportedDisplaysKHR vkGetDisplayPlaneSupportedDisplaysKHR_ = nullptr;
  PFN_vkGetDisplayModePropertiesKHR vkGetDisplayModePropertiesKHR_ = nullptr;
  PFN_vkCreateDisplayModeKHR vkCreateDisplayModeKHR_ = nullptr;
  PFN_vkGetDisplayPlaneCapabilitiesKHR vkGetDisplayPlaneCapabilitiesKHR_ = nullptr;
#if defined(VK_USE_PLATFORM_XLIB_KHR)
  PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR vkGetPhysicalDeviceXlibPresentationSupportKHR_ = nullptr;
#endif
#if defined(VK_USE_PLATFORM_XCB_KHR)
  PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR vkGetPhysicalDeviceXcbPresentationSupportKHR_ = nullptr;
#endif
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
  PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR vkGetPhysicalDeviceWaylandPresentationSupportKHR_ = nullptr;
#endif
#if defined(VK_USE_PLATFORM_MIR_KHR)
  PFN_vkGetPhysicalDeviceMirPresentationSupportKHR vkGetPhysicalDeviceMirPresentationSupportKHR_ = nullptr;
#endif
#if defined(VK_USE_PLATFORM_WIN32_KHR)
  PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR vkGetPhysicalDeviceWin32PresentationSupportKHR_ = nullptr;
#endif
  PFN_vkGetPhysicalDeviceExternalImageFormatPropertiesNV vkGetPhysicalDeviceExternalImageFormatPropertiesNV_ = nullptr;
  PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR_ = nullptr;
  PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2KHR_ = nullptr;
  PFN_vkGetPhysicalDeviceFormatProperties2KHR vkGetPhysicalDeviceFormatProperties2KHR_ = nullptr;
  PFN_vkGetPhysicalDeviceImageFormatProperties2KHR vkGetPhysicalDeviceImageFormatProperties2KHR_ = nullptr;
  PFN_vkGetPhysicalDeviceQueueFamilyProperties2KHR vkGetPhysicalDeviceQueueFamilyProperties2KHR_ = nullptr;
  PFN_vkGetPhysicalDeviceMemoryProperties2KHR vkGetPhysicalDeviceMemoryProperties2KHR_ = nullptr;
  PFN_vkGetPhysicalDeviceSparseImageFormatProperties2KHR vkGetPhysicalDeviceSparseImageFormatProperties2KHR_ = nullptr;
  PFN_vkGetPhysicalDeviceExternalBufferPropertiesKHR vkGetPhysicalDeviceExternalBufferPropertiesKHR_ = nullptr;
  PFN_vkGetPhysicalDeviceExternalSemaphorePropertiesKHR vkGetPhysicalDeviceExternalSemaphorePropertiesKHR_ = nullptr;
  PFN_vkGetPhysicalDeviceGeneratedCommandsPropertiesNVX vkGetPhysicalDeviceGeneratedCommandsPropertiesNVX_ = nullptr;
  PFN_vkReleaseDisplayEXT vkReleaseDisplayEXT_ = nullptr;
#if defined(VK_USE_PLATFORM_XLIB_XRANDR_EXT)
  PFN_vkAcquireXlibDisplayEXT vkAcquireXlibDisplayEXT_ = nullptr;
#endif
#if defined(VK_USE_PLATFORM_XLIB_XRANDR_EXT)
  PFN_vkGetRandROutputDisplayEXT vkGetRandROutputDisplayEXT_ = nullptr;
#endif
  PFN_vkGetPhysicalDeviceSurfaceCapabilities2EXT vkGetPhysicalDeviceSurfaceCapabilities2EXT_ = nullptr;
  PFN_vkGetPhysicalDeviceExternalFencePropertiesKHR vkGetPhysicalDeviceExternalFencePropertiesKHR_ = nullptr;
  PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR vkGetPhysicalDeviceSurfaceCapabilities2KHR_ = nullptr;
  PFN_vkGetPhysicalDeviceSurfaceFormats2KHR vkGetPhysicalDeviceSurfaceFormats2KHR_ = nullptr;
  PFN_vkGetPhysicalDeviceMultisamplePropertiesEXT vkGetPhysicalDeviceMultisamplePropertiesEXT_ = nullptr;
};

class DeviceFunctions {
public:
  VkDevice device() const { return device_; }
  PFN_vkVoidFunction vkGetDeviceProcAddr(const char* pName) const;
  void vkDestroyDevice(const VkAllocationCallbacks* pAllocator) const;
  void vkGetDeviceQueue(uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue) const;
  VkResult vkDeviceWaitIdle() const;
  VkResult vkAllocateMemory(const VkMemoryAllocateInfo* pAllocateInfo, const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory) const;
  void vkFreeMemory(VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator) const;
  VkResult vkMapMemory(VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** ppData) const;
  void vkUnmapMemory(VkDeviceMemory memory) const;
  VkResult vkFlushMappedMemoryRanges(uint32_t memoryRangeCount, const VkMappedMemoryRange* pMemoryRanges) const;
  VkResult vkInvalidateMappedMemoryRanges(uint32_t memoryRangeCount, const VkMappedMemoryRange* pMemoryRanges) const;
  void vkGetDeviceMemoryCommitment(VkDeviceMemory memory, VkDeviceSize* pCommittedMemoryInBytes) const;
  VkResult vkBindBufferMemory(VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset) const;
  VkResult vkBindImageMemory(VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset) const;
  void vkGetBufferMemoryRequirements(VkBuffer buffer, VkMemoryRequirements* pMemoryRequirements) const;
  void vkGetImageMemoryRequirements(VkImage image, VkMemoryRequirements* pMemoryRequirements) const;
  void vkGetImageSparseMemoryRequirements(VkImage image, uint32_t* pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements* pSparseMemoryRequirements) const;
  VkResult vkCreateFence(const VkFenceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkFence* pFence) const;
  void vkDestroyFence(VkFence fence, const VkAllocationCallbacks* pAllocator) const;
  VkResult vkResetFences(uint32_t fenceCount, const VkFence* pFences) const;
  VkResult vkGetFenceStatus(VkFence fence) const;
  VkResult vkWaitForFences(uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll, uint64_t timeout) const;
  VkResult vkCreateSemaphore(const VkSemaphoreCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore) const;
  void vkDestroySemaphore(VkSemaphore semaphore, const VkAllocationCallbacks* pAllocator) const;
  VkResult vkCreateEvent(const VkEventCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkEvent* pEvent) const;
  void vkDestroyEvent(VkEvent event, const VkAllocationCallbacks* pAllocator) const;
  VkResult vkGetEventStatus(VkEvent event) const;
  VkResult vkSetEvent(VkEvent event) const;
  VkResult vkResetEvent(VkEvent event) const;
  VkResult vkCreateQueryPool(const VkQueryPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkQueryPool* pQueryPool) const;
  void vkDestroyQueryPool(VkQueryPool queryPool, const VkAllocationCallbacks* pAllocator) const;
  VkResult vkGetQueryPoolResults(VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, size_t dataSize, void* pData, VkDeviceSize stride, VkQueryResultFlags flags) const;
  VkResult vkCreateBuffer(const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer) const;
  void vkDestroyBuffer(VkBuffer buffer, const VkAllocationCallbacks* pAllocator) const;
  VkResult vkCreateBufferView(const VkBufferViewCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBufferView* pView) const;
  void vkDestroyBufferView(VkBufferView bufferView, const VkAllocationCallbacks* pAllocator) const;
  VkResult vkCreateImage(const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImage* pImage) const;
  void vkDestroyImage(VkImage image, const VkAllocationCallbacks* pAllocator) const;
  void vkGetImageSubresourceLayout(VkImage image, const VkImageSubresource* pSubresource, VkSubresourceLayout* pLayout) const;
  VkResult vkCreateImageView(const VkImageViewCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImageView* pView) const;
  void vkDestroyImageView(VkImageView imageView, const VkAllocationCallbacks* pAllocator) const;
  VkResult vkCreateShaderModule(const VkShaderModuleCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule) const;
  void vkDestroyShaderModule(VkShaderModule shaderModule, const VkAllocationCallbacks* pAllocator) const;
  VkResult vkCreatePipelineCache(const VkPipelineCacheCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineCache* pPipelineCache) const;
  void vkDestroyPipelineCache(VkPipelineCache pipelineCache, const VkAllocationCallbacks* pAllocator) const;
  VkResult vkGetPipelineCacheData(VkPipelineCache pipelineCache, size_t* pDataSize, void* pData) const;
  VkResult vkMergePipelineCaches(VkPipelineCache dstCache, uint32_t srcCacheCount, const VkPipelineCache* pSrcCaches) const;
  VkResult vkCreateGraphicsPipelines(VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkGraphicsPipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines) const;
  VkResult vkCreateComputePipelines(VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkComputePipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines) const;
  void vkDestroyPipeline(VkPipeline pipeline, const VkAllocationCallbacks* pAllocator) const;
  VkResult vkCreatePipelineLayout(const VkPipelineLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout) const;
  void vkDestroyPipelineLayout(VkPipelineLayout pipelineLayout, const VkAllocationCallbacks* pAllocator) const;
  VkResult vkCreateSampler(const VkSamplerCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSampler* pSampler) const;
  void vkDestroySampler(VkSampler sampler, const VkAllocationCallbacks* pAllocator) const;
  VkResult vkCreateDescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorSetLayout* pSetLayout) const;
  void vkDestroyDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout, const VkAllocationCallbacks* pAllocator) const;
  VkResult vkCreateDescriptorPool(const VkDescriptorPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorPool* pDescriptorPool) const;
  void vkDestroyDescriptorPool(VkDescriptorPool descriptorPool, const VkAllocationCallbacks* pAllocator) const;
  VkResult vkResetDescriptorPool(VkDescriptorPool descriptorPool, VkDescriptorPoolResetFlags flags) const;
  VkResult vkAllocateDescriptorSets(const VkDescriptorSetAllocateInfo* pAllocateInfo, VkDescriptorSet* pDescriptorSets) const;
  VkResult vkFreeDescriptorSets(VkDescriptorPool descriptorPool, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets) const;
  void vkUpdateDescriptorSets(uint32_t descriptorWriteCount, const VkWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount, const VkCopyDescriptorSet* pDescriptorCopies) const;
  VkResult vkCreateFramebuffer(const VkFramebufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer) const;
  void vkDestroyFramebuffer(VkFramebuffer framebuffer, const VkAllocationCallbacks* pAllocator) const;
  VkResult vkCreateRenderPass(const VkRenderPassCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass) const;
  void vkDestroyRenderPass(VkRenderPass renderPass, const VkAllocationCallbacks* pAllocator) const;
  void vkGetRenderAreaGranularity(VkRenderPass renderPass, VkExtent2D* pGranularity) const;
  VkResult vkCreateCommandPool(const VkCommandPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool) const;
  void vkDestroyCommandPool(VkCommandPool commandPool, const VkAllocationCallbacks* pAllocator) const;
  VkResult vkResetCommandPool(VkCommandPool commandPool, VkCommandPoolResetFlags flags) const;
  VkResult vkAllocateCommandBuffers(const VkCommandBufferAllocateInfo* pAllocateInfo, VkCommandBuffer* pCommandBuffers) const;
  void vkFreeCommandBuffers(VkCommandPool commandPool, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers) const;
  VkResult vkCreateSwapchainKHR(const VkSwapchainCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain) const;
  void vkDestroySwapchainKHR(VkSwapchainKHR swapchain, const VkAllocationCallbacks* pAllocator) const;
  VkResult vkGetSwapchainImagesKHR(VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount, VkImage* pSwapchainImages) const;
  VkResult vkAcquireNextImageKHR(VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex) const;
  VkResult vkCreateSharedSwapchainsKHR(uint32_t swapchainCount, const VkSwapchainCreateInfoKHR* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchains) const;
  VkResult vkDebugMarkerSetObjectTagEXT(const VkDebugMarkerObjectTagInfoEXT* pTagInfo) const;
  VkResult vkDebugMarkerSetObjectNameEXT(const VkDebugMarkerObjectNameInfoEXT* pNameInfo) const;
  VkResult vkGetShaderInfoAMD(VkPipeline pipeline, VkShaderStageFlagBits shaderStage, VkShaderInfoTypeAMD infoType, size_t* pInfoSize, void* pInfo) const;
#if defined(VK_USE_PLATFORM_WIN32_KHR)
  VkResult vkGetMemoryWin32HandleNV(VkDeviceMemory memory, VkExternalMemoryHandleTypeFlagsNV handleType, HANDLE* pHandle) const;
#endif
  void vkGetDeviceGroupPeerMemoryFeaturesKHX(uint32_t heapIndex, uint32_t localDeviceIndex, uint32_t remoteDeviceIndex, VkPeerMemoryFeatureFlagsKHX* pPeerMemoryFeatures) const;
  void vkTrimCommandPoolKHR(VkCommandPool commandPool, VkCommandPoolTrimFlagsKHR flags) const;
#if defined(VK_USE_PLATFORM_WIN32_KHR)
  VkResult vkGetMemoryWin32HandleKHR(const VkMemoryGetWin32HandleInfoKHR* pGetWin32HandleInfo, HANDLE* pHandle) const;
#endif
#if defined(VK_USE_PLATFORM_WIN32_KHR)
  VkResult vkGetMemoryWin32HandlePropertiesKHR(VkExternalMemoryHandleTypeFlagBitsKHR handleType, HANDLE handle, VkMemoryWin32HandlePropertiesKHR* pMemoryWin32HandleProperties) const;
#endif
  VkResult vkGetMemoryFdKHR(const VkMemoryGetFdInfoKHR* pGetFdInfo, int* pFd) const;
  VkResult vkGetMemoryFdPropertiesKHR(VkExternalMemoryHandleTypeFlagBitsKHR handleType, int fd, VkMemoryFdPropertiesKHR* pMemoryFdProperties) const;
#if defined(VK_USE_PLATFORM_WIN32_KHR)
  VkResult vkImportSemaphoreWin32HandleKHR(const VkImportSemaphoreWin32HandleInfoKHR* pImportSemaphoreWin32HandleInfo) const;
#endif
#if defined(VK_USE_PLATFORM_WIN32_KHR)
  VkResult vkGetSemaphoreWin32HandleKHR(const VkSemaphoreGetWin32HandleInfoKHR* pGetWin32HandleInfo, HANDLE* pHandle) const;
#endif
  VkResult vkImportSemaphoreFdKHR(const VkImportSemaphoreFdInfoKHR* pImportSemaphoreFdInfo) const;
  VkResult vkGetSemaphoreFdKHR(const VkSemaphoreGetFdInfoKHR* pGetFdInfo, int* pFd) const;
  VkResult vkCreateDescriptorUpdateTemplateKHR(const VkDescriptorUpdateTemplateCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorUpdateTemplateKHR* pDescriptorUpdateTemplate) const;
  void vkDestroyDescriptorUpdateTemplateKHR(VkDescriptorUpdateTemplateKHR descriptorUpdateTemplate, const VkAllocationCallbacks* pAllocator) const;
  void vkUpdateDescriptorSetWithTemplateKHR(VkDescriptorSet descriptorSet, VkDescriptorUpdateTemplateKHR descriptorUpdateTemplate, const void* pData) const;
  VkResult vkCreateIndirectCommandsLayoutNVX(const VkIndirectCommandsLayoutCreateInfoNVX* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkIndirectCommandsLayoutNVX* pIndirectCommandsLayout) const;
  void vkDestroyIndirectCommandsLayoutNVX(VkIndirectCommandsLayoutNVX indirectCommandsLayout, const VkAllocationCallbacks* pAllocator) const;
  VkResult vkCreateObjectTableNVX(const VkObjectTableCreateInfoNVX* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkObjectTableNVX* pObjectTable) const;
  void vkDestroyObjectTableNVX(VkObjectTableNVX objectTable, const VkAllocationCallbacks* pAllocator) const;
  VkResult vkRegisterObjectsNVX(VkObjectTableNVX objectTable, uint32_t objectCount, const VkObjectTableEntryNVX* const* ppObjectTableEntries, const uint32_t* pObjectIndices) const;
  VkResult vkUnregisterObjectsNVX(VkObjectTableNVX objectTable, uint32_t objectCount, const VkObjectEntryTypeNVX* pObjectEntryTypes, const uint32_t* pObjectIndices) const;
  VkResult vkDisplayPowerControlEXT(VkDisplayKHR display, const VkDisplayPowerInfoEXT* pDisplayPowerInfo) const;
  VkResult vkRegisterDeviceEventEXT(const VkDeviceEventInfoEXT* pDeviceEventInfo, const VkAllocationCallbacks* pAllocator, VkFence* pFence) const;
  VkResult vkRegisterDisplayEventEXT(VkDisplayKHR display, const VkDisplayEventInfoEXT* pDisplayEventInfo, const VkAllocationCallbacks* pAllocator, VkFence* pFence) const;
  VkResult vkGetSwapchainCounterEXT(VkSwapchainKHR swapchain, VkSurfaceCounterFlagBitsEXT counter, uint64_t* pCounterValue) const;
  VkResult vkGetRefreshCycleDurationGOOGLE(VkSwapchainKHR swapchain, VkRefreshCycleDurationGOOGLE* pDisplayTimingProperties) const;
  VkResult vkGetPastPresentationTimingGOOGLE(VkSwapchainKHR swapchain, uint32_t* pPresentationTimingCount, VkPastPresentationTimingGOOGLE* pPresentationTimings) const;
  void vkSetHdrMetadataEXT(uint32_t swapchainCount, const VkSwapchainKHR* pSwapchains, const VkHdrMetadataEXT* pMetadata) const;
  VkResult vkGetSwapchainStatusKHR(VkSwapchainKHR swapchain) const;
#if defined(VK_USE_PLATFORM_WIN32_KHR)
  VkResult vkImportFenceWin32HandleKHR(const VkImportFenceWin32HandleInfoKHR* pImportFenceWin32HandleInfo) const;
#endif
#if defined(VK_USE_PLATFORM_WIN32_KHR)
  VkResult vkGetFenceWin32HandleKHR(const VkFenceGetWin32HandleInfoKHR* pGetWin32HandleInfo, HANDLE* pHandle) const;
#endif
  VkResult vkImportFenceFdKHR(const VkImportFenceFdInfoKHR* pImportFenceFdInfo) const;
  VkResult vkGetFenceFdKHR(const VkFenceGetFdInfoKHR* pGetFdInfo, int* pFd) const;
  void vkGetImageMemoryRequirements2KHR(const VkImageMemoryRequirementsInfo2KHR* pInfo, VkMemoryRequirements2KHR* pMemoryRequirements) const;
  void vkGetBufferMemoryRequirements2KHR(const VkBufferMemoryRequirementsInfo2KHR* pInfo, VkMemoryRequirements2KHR* pMemoryRequirements) const;
  void vkGetImageSparseMemoryRequirements2KHR(const VkImageSparseMemoryRequirementsInfo2KHR* pInfo, uint32_t* pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements2KHR* pSparseMemoryRequirements) const;
  VkResult vkCreateSamplerYcbcrConversionKHR(const VkSamplerYcbcrConversionCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSamplerYcbcrConversionKHR* pYcbcrConversion) const;
  void vkDestroySamplerYcbcrConversionKHR(VkSamplerYcbcrConversionKHR ycbcrConversion, const VkAllocationCallbacks* pAllocator) const;
  VkResult vkBindBufferMemory2KHR(uint32_t bindInfoCount, const VkBindBufferMemoryInfoKHR* pBindInfos) const;
  VkResult vkBindImageMemory2KHR(uint32_t bindInfoCount, const VkBindImageMemoryInfoKHR* pBindInfos) const;
  VkResult vkCreateValidationCacheEXT(const VkValidationCacheCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkValidationCacheEXT* pValidationCache) const;
  void vkDestroyValidationCacheEXT(VkValidationCacheEXT validationCache, const VkAllocationCallbacks* pAllocator) const;
  VkResult vkMergeValidationCachesEXT(VkValidationCacheEXT dstCache, uint32_t srcCacheCount, const VkValidationCacheEXT* pSrcCaches) const;
  VkResult vkGetValidationCacheDataEXT(VkValidationCacheEXT validationCache, size_t* pDataSize, void* pData) const;

protected:
  DeviceFunctions(VkDevice device, InstanceFunctions* instance);

private:
  VkDevice device_ = VK_NULL_HANDLE;
  PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr_ = nullptr;
  PFN_vkDestroyDevice vkDestroyDevice_ = nullptr;
  PFN_vkGetDeviceQueue vkGetDeviceQueue_ = nullptr;
  PFN_vkDeviceWaitIdle vkDeviceWaitIdle_ = nullptr;
  PFN_vkAllocateMemory vkAllocateMemory_ = nullptr;
  PFN_vkFreeMemory vkFreeMemory_ = nullptr;
  PFN_vkMapMemory vkMapMemory_ = nullptr;
  PFN_vkUnmapMemory vkUnmapMemory_ = nullptr;
  PFN_vkFlushMappedMemoryRanges vkFlushMappedMemoryRanges_ = nullptr;
  PFN_vkInvalidateMappedMemoryRanges vkInvalidateMappedMemoryRanges_ = nullptr;
  PFN_vkGetDeviceMemoryCommitment vkGetDeviceMemoryCommitment_ = nullptr;
  PFN_vkBindBufferMemory vkBindBufferMemory_ = nullptr;
  PFN_vkBindImageMemory vkBindImageMemory_ = nullptr;
  PFN_vkGetBufferMemoryRequirements vkGetBufferMemoryRequirements_ = nullptr;
  PFN_vkGetImageMemoryRequirements vkGetImageMemoryRequirements_ = nullptr;
  PFN_vkGetImageSparseMemoryRequirements vkGetImageSparseMemoryRequirements_ = nullptr;
  PFN_vkCreateFence vkCreateFence_ = nullptr;
  PFN_vkDestroyFence vkDestroyFence_ = nullptr;
  PFN_vkResetFences vkResetFences_ = nullptr;
  PFN_vkGetFenceStatus vkGetFenceStatus_ = nullptr;
  PFN_vkWaitForFences vkWaitForFences_ = nullptr;
  PFN_vkCreateSemaphore vkCreateSemaphore_ = nullptr;
  PFN_vkDestroySemaphore vkDestroySemaphore_ = nullptr;
  PFN_vkCreateEvent vkCreateEvent_ = nullptr;
  PFN_vkDestroyEvent vkDestroyEvent_ = nullptr;
  PFN_vkGetEventStatus vkGetEventStatus_ = nullptr;
  PFN_vkSetEvent vkSetEvent_ = nullptr;
  PFN_vkResetEvent vkResetEvent_ = nullptr;
  PFN_vkCreateQueryPool vkCreateQueryPool_ = nullptr;
  PFN_vkDestroyQueryPool vkDestroyQueryPool_ = nullptr;
  PFN_vkGetQueryPoolResults vkGetQueryPoolResults_ = nullptr;
  PFN_vkCreateBuffer vkCreateBuffer_ = nullptr;
  PFN_vkDestroyBuffer vkDestroyBuffer_ = nullptr;
  PFN_vkCreateBufferView vkCreateBufferView_ = nullptr;
  PFN_vkDestroyBufferView vkDestroyBufferView_ = nullptr;
  PFN_vkCreateImage vkCreateImage_ = nullptr;
  PFN_vkDestroyImage vkDestroyImage_ = nullptr;
  PFN_vkGetImageSubresourceLayout vkGetImageSubresourceLayout_ = nullptr;
  PFN_vkCreateImageView vkCreateImageView_ = nullptr;
  PFN_vkDestroyImageView vkDestroyImageView_ = nullptr;
  PFN_vkCreateShaderModule vkCreateShaderModule_ = nullptr;
  PFN_vkDestroyShaderModule vkDestroyShaderModule_ = nullptr;
  PFN_vkCreatePipelineCache vkCreatePipelineCache_ = nullptr;
  PFN_vkDestroyPipelineCache vkDestroyPipelineCache_ = nullptr;
  PFN_vkGetPipelineCacheData vkGetPipelineCacheData_ = nullptr;
  PFN_vkMergePipelineCaches vkMergePipelineCaches_ = nullptr;
  PFN_vkCreateGraphicsPipelines vkCreateGraphicsPipelines_ = nullptr;
  PFN_vkCreateComputePipelines vkCreateComputePipelines_ = nullptr;
  PFN_vkDestroyPipeline vkDestroyPipeline_ = nullptr;
  PFN_vkCreatePipelineLayout vkCreatePipelineLayout_ = nullptr;
  PFN_vkDestroyPipelineLayout vkDestroyPipelineLayout_ = nullptr;
  PFN_vkCreateSampler vkCreateSampler_ = nullptr;
  PFN_vkDestroySampler vkDestroySampler_ = nullptr;
  PFN_vkCreateDescriptorSetLayout vkCreateDescriptorSetLayout_ = nullptr;
  PFN_vkDestroyDescriptorSetLayout vkDestroyDescriptorSetLayout_ = nullptr;
  PFN_vkCreateDescriptorPool vkCreateDescriptorPool_ = nullptr;
  PFN_vkDestroyDescriptorPool vkDestroyDescriptorPool_ = nullptr;
  PFN_vkResetDescriptorPool vkResetDescriptorPool_ = nullptr;
  PFN_vkAllocateDescriptorSets vkAllocateDescriptorSets_ = nullptr;
  PFN_vkFreeDescriptorSets vkFreeDescriptorSets_ = nullptr;
  PFN_vkUpdateDescriptorSets vkUpdateDescriptorSets_ = nullptr;
  PFN_vkCreateFramebuffer vkCreateFramebuffer_ = nullptr;
  PFN_vkDestroyFramebuffer vkDestroyFramebuffer_ = nullptr;
  PFN_vkCreateRenderPass vkCreateRenderPass_ = nullptr;
  PFN_vkDestroyRenderPass vkDestroyRenderPass_ = nullptr;
  PFN_vkGetRenderAreaGranularity vkGetRenderAreaGranularity_ = nullptr;
  PFN_vkCreateCommandPool vkCreateCommandPool_ = nullptr;
  PFN_vkDestroyCommandPool vkDestroyCommandPool_ = nullptr;
  PFN_vkResetCommandPool vkResetCommandPool_ = nullptr;
  PFN_vkAllocateCommandBuffers vkAllocateCommandBuffers_ = nullptr;
  PFN_vkFreeCommandBuffers vkFreeCommandBuffers_ = nullptr;
  PFN_vkCreateSwapchainKHR vkCreateSwapchainKHR_ = nullptr;
  PFN_vkDestroySwapchainKHR vkDestroySwapchainKHR_ = nullptr;
  PFN_vkGetSwapchainImagesKHR vkGetSwapchainImagesKHR_ = nullptr;
  PFN_vkAcquireNextImageKHR vkAcquireNextImageKHR_ = nullptr;
  PFN_vkCreateSharedSwapchainsKHR vkCreateSharedSwapchainsKHR_ = nullptr;
  PFN_vkDebugMarkerSetObjectTagEXT vkDebugMarkerSetObjectTagEXT_ = nullptr;
  PFN_vkDebugMarkerSetObjectNameEXT vkDebugMarkerSetObjectNameEXT_ = nullptr;
  PFN_vkGetShaderInfoAMD vkGetShaderInfoAMD_ = nullptr;
#if defined(VK_USE_PLATFORM_WIN32_KHR)
  PFN_vkGetMemoryWin32HandleNV vkGetMemoryWin32HandleNV_ = nullptr;
#endif
  PFN_vkGetDeviceGroupPeerMemoryFeaturesKHX vkGetDeviceGroupPeerMemoryFeaturesKHX_ = nullptr;
  PFN_vkTrimCommandPoolKHR vkTrimCommandPoolKHR_ = nullptr;
#if defined(VK_USE_PLATFORM_WIN32_KHR)
  PFN_vkGetMemoryWin32HandleKHR vkGetMemoryWin32HandleKHR_ = nullptr;
#endif
#if defined(VK_USE_PLATFORM_WIN32_KHR)
  PFN_vkGetMemoryWin32HandlePropertiesKHR vkGetMemoryWin32HandlePropertiesKHR_ = nullptr;
#endif
  PFN_vkGetMemoryFdKHR vkGetMemoryFdKHR_ = nullptr;
  PFN_vkGetMemoryFdPropertiesKHR vkGetMemoryFdPropertiesKHR_ = nullptr;
#if defined(VK_USE_PLATFORM_WIN32_KHR)
  PFN_vkImportSemaphoreWin32HandleKHR vkImportSemaphoreWin32HandleKHR_ = nullptr;
#endif
#if defined(VK_USE_PLATFORM_WIN32_KHR)
  PFN_vkGetSemaphoreWin32HandleKHR vkGetSemaphoreWin32HandleKHR_ = nullptr;
#endif
  PFN_vkImportSemaphoreFdKHR vkImportSemaphoreFdKHR_ = nullptr;
  PFN_vkGetSemaphoreFdKHR vkGetSemaphoreFdKHR_ = nullptr;
  PFN_vkCreateDescriptorUpdateTemplateKHR vkCreateDescriptorUpdateTemplateKHR_ = nullptr;
  PFN_vkDestroyDescriptorUpdateTemplateKHR vkDestroyDescriptorUpdateTemplateKHR_ = nullptr;
  PFN_vkUpdateDescriptorSetWithTemplateKHR vkUpdateDescriptorSetWithTemplateKHR_ = nullptr;
  PFN_vkCreateIndirectCommandsLayoutNVX vkCreateIndirectCommandsLayoutNVX_ = nullptr;
  PFN_vkDestroyIndirectCommandsLayoutNVX vkDestroyIndirectCommandsLayoutNVX_ = nullptr;
  PFN_vkCreateObjectTableNVX vkCreateObjectTableNVX_ = nullptr;
  PFN_vkDestroyObjectTableNVX vkDestroyObjectTableNVX_ = nullptr;
  PFN_vkRegisterObjectsNVX vkRegisterObjectsNVX_ = nullptr;
  PFN_vkUnregisterObjectsNVX vkUnregisterObjectsNVX_ = nullptr;
  PFN_vkDisplayPowerControlEXT vkDisplayPowerControlEXT_ = nullptr;
  PFN_vkRegisterDeviceEventEXT vkRegisterDeviceEventEXT_ = nullptr;
  PFN_vkRegisterDisplayEventEXT vkRegisterDisplayEventEXT_ = nullptr;
  PFN_vkGetSwapchainCounterEXT vkGetSwapchainCounterEXT_ = nullptr;
  PFN_vkGetRefreshCycleDurationGOOGLE vkGetRefreshCycleDurationGOOGLE_ = nullptr;
  PFN_vkGetPastPresentationTimingGOOGLE vkGetPastPresentationTimingGOOGLE_ = nullptr;
  PFN_vkSetHdrMetadataEXT vkSetHdrMetadataEXT_ = nullptr;
  PFN_vkGetSwapchainStatusKHR vkGetSwapchainStatusKHR_ = nullptr;
#if defined(VK_USE_PLATFORM_WIN32_KHR)
  PFN_vkImportFenceWin32HandleKHR vkImportFenceWin32HandleKHR_ = nullptr;
#endif
#if defined(VK_USE_PLATFORM_WIN32_KHR)
  PFN_vkGetFenceWin32HandleKHR vkGetFenceWin32HandleKHR_ = nullptr;
#endif
  PFN_vkImportFenceFdKHR vkImportFenceFdKHR_ = nullptr;
  PFN_vkGetFenceFdKHR vkGetFenceFdKHR_ = nullptr;
  PFN_vkGetImageMemoryRequirements2KHR vkGetImageMemoryRequirements2KHR_ = nullptr;
  PFN_vkGetBufferMemoryRequirements2KHR vkGetBufferMemoryRequirements2KHR_ = nullptr;
  PFN_vkGetImageSparseMemoryRequirements2KHR vkGetImageSparseMemoryRequirements2KHR_ = nullptr;
  PFN_vkCreateSamplerYcbcrConversionKHR vkCreateSamplerYcbcrConversionKHR_ = nullptr;
  PFN_vkDestroySamplerYcbcrConversionKHR vkDestroySamplerYcbcrConversionKHR_ = nullptr;
  PFN_vkBindBufferMemory2KHR vkBindBufferMemory2KHR_ = nullptr;
  PFN_vkBindImageMemory2KHR vkBindImageMemory2KHR_ = nullptr;
  PFN_vkCreateValidationCacheEXT vkCreateValidationCacheEXT_ = nullptr;
  PFN_vkDestroyValidationCacheEXT vkDestroyValidationCacheEXT_ = nullptr;
  PFN_vkMergeValidationCachesEXT vkMergeValidationCachesEXT_ = nullptr;
  PFN_vkGetValidationCacheDataEXT vkGetValidationCacheDataEXT_ = nullptr;
};

class QueueFunctions {
public:
  VkQueue queue() const { return queue_; }
  VkResult vkQueueSubmit(uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence) const;
  VkResult vkQueueWaitIdle() const;
  VkResult vkQueueBindSparse(uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo, VkFence fence) const;
  VkResult vkQueuePresentKHR(const VkPresentInfoKHR* pPresentInfo) const;

protected:
  QueueFunctions(VkQueue queue, DeviceFunctions* device);

private:
  VkQueue queue_ = VK_NULL_HANDLE;
  PFN_vkQueueSubmit vkQueueSubmit_ = nullptr;
  PFN_vkQueueWaitIdle vkQueueWaitIdle_ = nullptr;
  PFN_vkQueueBindSparse vkQueueBindSparse_ = nullptr;
  PFN_vkQueuePresentKHR vkQueuePresentKHR_ = nullptr;
};

class CommandBufferFunctions {
public:
  VkCommandBuffer command_buffer() const { return command_buffer_; }
  VkResult vkBeginCommandBuffer(const VkCommandBufferBeginInfo* pBeginInfo) const;
  VkResult vkEndCommandBuffer() const;
  VkResult vkResetCommandBuffer(VkCommandBufferResetFlags flags) const;
  void vkCmdBindPipeline(VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline) const;
  void vkCmdSetViewport(uint32_t firstViewport, uint32_t viewportCount, const VkViewport* pViewports) const;
  void vkCmdSetScissor(uint32_t firstScissor, uint32_t scissorCount, const VkRect2D* pScissors) const;
  void vkCmdSetLineWidth(float lineWidth) const;
  void vkCmdSetDepthBias(float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor) const;
  void vkCmdSetBlendConstants(const float blendConstants[4]) const;
  void vkCmdSetDepthBounds(float minDepthBounds, float maxDepthBounds) const;
  void vkCmdSetStencilCompareMask(VkStencilFaceFlags faceMask, uint32_t compareMask) const;
  void vkCmdSetStencilWriteMask(VkStencilFaceFlags faceMask, uint32_t writeMask) const;
  void vkCmdSetStencilReference(VkStencilFaceFlags faceMask, uint32_t reference) const;
  void vkCmdBindDescriptorSets(VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets) const;
  void vkCmdBindIndexBuffer(VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType) const;
  void vkCmdBindVertexBuffers(uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets) const;
  void vkCmdDraw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) const;
  void vkCmdDrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) const;
  void vkCmdDrawIndirect(VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride) const;
  void vkCmdDrawIndexedIndirect(VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride) const;
  void vkCmdDispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) const;
  void vkCmdDispatchIndirect(VkBuffer buffer, VkDeviceSize offset) const;
  void vkCmdCopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferCopy* pRegions) const;
  void vkCmdCopyImage(VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy* pRegions) const;
  void vkCmdBlitImage(VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit* pRegions, VkFilter filter) const;
  void vkCmdCopyBufferToImage(VkBuffer srcBuffer, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy* pRegions) const;
  void vkCmdCopyImageToBuffer(VkImage srcImage, VkImageLayout srcImageLayout, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions) const;
  void vkCmdUpdateBuffer(VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize dataSize, const void* pData) const;
  void vkCmdFillBuffer(VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size, uint32_t data) const;
  void vkCmdClearColorImage(VkImage image, VkImageLayout imageLayout, const VkClearColorValue* pColor, uint32_t rangeCount, const VkImageSubresourceRange* pRanges) const;
  void vkCmdClearDepthStencilImage(VkImage image, VkImageLayout imageLayout, const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount, const VkImageSubresourceRange* pRanges) const;
  void vkCmdClearAttachments(uint32_t attachmentCount, const VkClearAttachment* pAttachments, uint32_t rectCount, const VkClearRect* pRects) const;
  void vkCmdResolveImage(VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageResolve* pRegions) const;
  void vkCmdSetEvent(VkEvent event, VkPipelineStageFlags stageMask) const;
  void vkCmdResetEvent(VkEvent event, VkPipelineStageFlags stageMask) const;
  void vkCmdWaitEvents(uint32_t eventCount, const VkEvent* pEvents, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers) const;
  void vkCmdPipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers) const;
  void vkCmdBeginQuery(VkQueryPool queryPool, uint32_t query, VkQueryControlFlags flags) const;
  void vkCmdEndQuery(VkQueryPool queryPool, uint32_t query) const;
  void vkCmdResetQueryPool(VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount) const;
  void vkCmdWriteTimestamp(VkPipelineStageFlagBits pipelineStage, VkQueryPool queryPool, uint32_t query) const;
  void vkCmdCopyQueryPoolResults(VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride, VkQueryResultFlags flags) const;
  void vkCmdPushConstants(VkPipelineLayout layout, VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* pValues) const;
  void vkCmdBeginRenderPass(const VkRenderPassBeginInfo* pRenderPassBegin, VkSubpassContents contents) const;
  void vkCmdNextSubpass(VkSubpassContents contents) const;
  void vkCmdEndRenderPass() const;
  void vkCmdExecuteCommands(uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers) const;
  void vkCmdDebugMarkerBeginEXT(const VkDebugMarkerMarkerInfoEXT* pMarkerInfo) const;
  void vkCmdDebugMarkerEndEXT() const;
  void vkCmdDebugMarkerInsertEXT(const VkDebugMarkerMarkerInfoEXT* pMarkerInfo) const;
  void vkCmdDrawIndirectCountAMD(VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) const;
  void vkCmdDrawIndexedIndirectCountAMD(VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) const;
  void vkCmdSetDeviceMaskKHX(uint32_t deviceMask) const;
  void vkCmdDispatchBaseKHX(uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) const;
  void vkCmdPushDescriptorSetKHR(VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount, const VkWriteDescriptorSet* pDescriptorWrites) const;
  void vkCmdPushDescriptorSetWithTemplateKHR(VkDescriptorUpdateTemplateKHR descriptorUpdateTemplate, VkPipelineLayout layout, uint32_t set, const void* pData) const;
  void vkCmdProcessCommandsNVX(const VkCmdProcessCommandsInfoNVX* pProcessCommandsInfo) const;
  void vkCmdReserveSpaceForCommandsNVX(const VkCmdReserveSpaceForCommandsInfoNVX* pReserveSpaceInfo) const;
  void vkCmdSetViewportWScalingNV(uint32_t firstViewport, uint32_t viewportCount, const VkViewportWScalingNV* pViewportWScalings) const;
  void vkCmdSetDiscardRectangleEXT(uint32_t firstDiscardRectangle, uint32_t discardRectangleCount, const VkRect2D* pDiscardRectangles) const;
  void vkCmdSetSampleLocationsEXT(const VkSampleLocationsInfoEXT* pSampleLocationsInfo) const;

protected:
  CommandBufferFunctions(VkCommandBuffer command_buffer, DeviceFunctions* device);

private:
  VkCommandBuffer command_buffer_ = VK_NULL_HANDLE;
  PFN_vkBeginCommandBuffer vkBeginCommandBuffer_ = nullptr;
  PFN_vkEndCommandBuffer vkEndCommandBuffer_ = nullptr;
  PFN_vkResetCommandBuffer vkResetCommandBuffer_ = nullptr;
  PFN_vkCmdBindPipeline vkCmdBindPipeline_ = nullptr;
  PFN_vkCmdSetViewport vkCmdSetViewport_ = nullptr;
  PFN_vkCmdSetScissor vkCmdSetScissor_ = nullptr;
  PFN_vkCmdSetLineWidth vkCmdSetLineWidth_ = nullptr;
  PFN_vkCmdSetDepthBias vkCmdSetDepthBias_ = nullptr;
  PFN_vkCmdSetBlendConstants vkCmdSetBlendConstants_ = nullptr;
  PFN_vkCmdSetDepthBounds vkCmdSetDepthBounds_ = nullptr;
  PFN_vkCmdSetStencilCompareMask vkCmdSetStencilCompareMask_ = nullptr;
  PFN_vkCmdSetStencilWriteMask vkCmdSetStencilWriteMask_ = nullptr;
  PFN_vkCmdSetStencilReference vkCmdSetStencilReference_ = nullptr;
  PFN_vkCmdBindDescriptorSets vkCmdBindDescriptorSets_ = nullptr;
  PFN_vkCmdBindIndexBuffer vkCmdBindIndexBuffer_ = nullptr;
  PFN_vkCmdBindVertexBuffers vkCmdBindVertexBuffers_ = nullptr;
  PFN_vkCmdDraw vkCmdDraw_ = nullptr;
  PFN_vkCmdDrawIndexed vkCmdDrawIndexed_ = nullptr;
  PFN_vkCmdDrawIndirect vkCmdDrawIndirect_ = nullptr;
  PFN_vkCmdDrawIndexedIndirect vkCmdDrawIndexedIndirect_ = nullptr;
  PFN_vkCmdDispatch vkCmdDispatch_ = nullptr;
  PFN_vkCmdDispatchIndirect vkCmdDispatchIndirect_ = nullptr;
  PFN_vkCmdCopyBuffer vkCmdCopyBuffer_ = nullptr;
  PFN_vkCmdCopyImage vkCmdCopyImage_ = nullptr;
  PFN_vkCmdBlitImage vkCmdBlitImage_ = nullptr;
  PFN_vkCmdCopyBufferToImage vkCmdCopyBufferToImage_ = nullptr;
  PFN_vkCmdCopyImageToBuffer vkCmdCopyImageToBuffer_ = nullptr;
  PFN_vkCmdUpdateBuffer vkCmdUpdateBuffer_ = nullptr;
  PFN_vkCmdFillBuffer vkCmdFillBuffer_ = nullptr;
  PFN_vkCmdClearColorImage vkCmdClearColorImage_ = nullptr;
  PFN_vkCmdClearDepthStencilImage vkCmdClearDepthStencilImage_ = nullptr;
  PFN_vkCmdClearAttachments vkCmdClearAttachments_ = nullptr;
  PFN_vkCmdResolveImage vkCmdResolveImage_ = nullptr;
  PFN_vkCmdSetEvent vkCmdSetEvent_ = nullptr;
  PFN_vkCmdResetEvent vkCmdResetEvent_ = nullptr;
  PFN_vkCmdWaitEvents vkCmdWaitEvents_ = nullptr;
  PFN_vkCmdPipelineBarrier vkCmdPipelineBarrier_ = nullptr;
  PFN_vkCmdBeginQuery vkCmdBeginQuery_ = nullptr;
  PFN_vkCmdEndQuery vkCmdEndQuery_ = nullptr;
  PFN_vkCmdResetQueryPool vkCmdResetQueryPool_ = nullptr;
  PFN_vkCmdWriteTimestamp vkCmdWriteTimestamp_ = nullptr;
  PFN_vkCmdCopyQueryPoolResults vkCmdCopyQueryPoolResults_ = nullptr;
  PFN_vkCmdPushConstants vkCmdPushConstants_ = nullptr;
  PFN_vkCmdBeginRenderPass vkCmdBeginRenderPass_ = nullptr;
  PFN_vkCmdNextSubpass vkCmdNextSubpass_ = nullptr;
  PFN_vkCmdEndRenderPass vkCmdEndRenderPass_ = nullptr;
  PFN_vkCmdExecuteCommands vkCmdExecuteCommands_ = nullptr;
  PFN_vkCmdDebugMarkerBeginEXT vkCmdDebugMarkerBeginEXT_ = nullptr;
  PFN_vkCmdDebugMarkerEndEXT vkCmdDebugMarkerEndEXT_ = nullptr;
  PFN_vkCmdDebugMarkerInsertEXT vkCmdDebugMarkerInsertEXT_ = nullptr;
  PFN_vkCmdDrawIndirectCountAMD vkCmdDrawIndirectCountAMD_ = nullptr;
  PFN_vkCmdDrawIndexedIndirectCountAMD vkCmdDrawIndexedIndirectCountAMD_ = nullptr;
  PFN_vkCmdSetDeviceMaskKHX vkCmdSetDeviceMaskKHX_ = nullptr;
  PFN_vkCmdDispatchBaseKHX vkCmdDispatchBaseKHX_ = nullptr;
  PFN_vkCmdPushDescriptorSetKHR vkCmdPushDescriptorSetKHR_ = nullptr;
  PFN_vkCmdPushDescriptorSetWithTemplateKHR vkCmdPushDescriptorSetWithTemplateKHR_ = nullptr;
  PFN_vkCmdProcessCommandsNVX vkCmdProcessCommandsNVX_ = nullptr;
  PFN_vkCmdReserveSpaceForCommandsNVX vkCmdReserveSpaceForCommandsNVX_ = nullptr;
  PFN_vkCmdSetViewportWScalingNV vkCmdSetViewportWScalingNV_ = nullptr;
  PFN_vkCmdSetDiscardRectangleEXT vkCmdSetDiscardRectangleEXT_ = nullptr;
  PFN_vkCmdSetSampleLocationsEXT vkCmdSetSampleLocationsEXT_ = nullptr;
};

} // vkgen

#endif // VK_DISPATCH_TABLES_INCLUDE

