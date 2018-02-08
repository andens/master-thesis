#include "swapchain.h"

#include "device.h"
#include "physical_device.h"
#include "presentation_surface.h"

using namespace std;

namespace vk {

Swapchain::Swapchain(weak_ptr<Device> device,
    weak_ptr<PresentationSurface> surface)
  : device_(device), surface_(surface) {
  resize();
}

Swapchain::~Swapchain() {
  for (auto view : image_views_) {
    if (view) {
      device_.lock()->vkDestroyImageView(view, nullptr);
    }
  }
  image_views_.clear();

  if (swapchain_) {
    device_.lock()->vkDestroySwapchainKHR(swapchain_, nullptr);
    swapchain_ = VK_NULL_HANDLE;
  }
}

VkImage Swapchain::image(uint32_t index) const {
  if (index < images_.size()) {
    return images_[index];
  }
  return VK_NULL_HANDLE;
}

void Swapchain::resize() {
  auto device = device_.lock();

  if (device->device()) {
    device->vkDeviceWaitIdle();
  }
  else {
    throw runtime_error("Swapchain: cannot create swapchain because device is invalid!");
  }

  for (auto view : image_views_) {
    if (view) {
      device->vkDestroyImageView(view, nullptr);
    }
  }
  image_views_.clear();

  // Validation layer complains if this check is not made. I have made an OS
  // check when choosing what physical device to use, so presentation should be
  // supported for this physical device and this should never fail.
  VkBool32 supported = VK_FALSE;
  device->physical_device()->vkGetPhysicalDeviceSurfaceSupportKHR(0, surface_.lock()->surface(), &supported);
  if (supported == VK_FALSE) {
    throw runtime_error("Presentation surface not supported.");
  }

  // Decide a suitable format for the swapchain images
  auto surface_formats = device->physical_device()->supported_surface_formats(surface_.lock()->surface());

  // If the format list contains just one entry of VK_FORMAT_UNDEFINED, the
  // surface has no preferred format. Otherwise, at least one supported format
  // will be returned. The swapchain must know the color space in order to do
  // any eventual color space transformations before presenting an image. From
  // spec: format specifies encoding of pixels (hardware can convert to/from
  // sRGB automatically), whereas color space determines how the presentation
  // engine interprets the data. In other words: a non-sRGB format with an SRGB
  // color space means that the hardware won't do sRGB conversions, but the
  // swapchain will still treat the data as sRGB, so in those cases we need to
  // do conversion ourselves. 
  VkFormat color_format;
  VkColorSpaceKHR color_space;
  if (surface_formats.size() == 1 && surface_formats[0].format == VK_FORMAT_UNDEFINED) {
    // Important: I don't blit but rather just copy raw data from LDR image to
    // swapchain. If format changes, be sure to change LDR format accordingly.
    color_format = VK_FORMAT_R8G8B8A8_UNORM;
    color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
  }
  else {
    // In this case I don't quite know how to do proper matching with color
    // space. I could just select the first pair that has SRGB color space,
    // but that doesn't guarantee that the format itself is SRGB and I don't
    // know how to find that out without looping over hardcoded values...
    // Note: If I match and things look bright, consider whether textures
    // with data in SRGB are still created as SRGB images.
    //color_format = surface_formats[0].format;
    //color_space = surface_formats[0].colorSpace;

    // I'm deliberately presenting in "wrong" mode because it looks better.
    // Colors are more distinct and image doesn't look washed out. So much for
    // presentation engine interpreting stuff correctly.
    color_format = VK_FORMAT_R8G8B8A8_UNORM;
    color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
  }
  format_ = color_format;

  // Surface capabilities is consulted for the number of buffers we can ask
  // for, the resolution to use, available usage flags, and transforms.
  VkSurfaceCapabilitiesKHR surface_capabilities = {};
  device->physical_device()->vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
      surface_.lock()->surface(), &surface_capabilities);

  // We look for triple buffering. If maxImageCount == 0, there is no limit in
  // the number of images. minImageCount is the number of images that must be
  // in the swapchain for vkAcquireNextImageKHR to work.
  uint32_t desired_image_count = surface_capabilities.minImageCount + 1;
  if (desired_image_count < surface_capabilities.minImageCount) {
    desired_image_count = surface_capabilities.minImageCount;
  }
  else if (surface_capabilities.maxImageCount != 0 && desired_image_count > surface_capabilities.maxImageCount) {
    desired_image_count = surface_capabilities.maxImageCount;
  }

  VkExtent2D surface_resolution = surface_capabilities.currentExtent;

  // Color attachment flag is always supported, but if we want to use other
  // usage flags we must make sure that they are available. Here we check for
  // image transfer, allowing clear operations (vkCmdClearColorImage requires
  // image layout to be general or transfer dst).
  VkImageUsageFlags usage_flags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
  if ((surface_capabilities.supportedUsageFlags & usage_flags) != usage_flags) {
    throw runtime_error("Swapchain does not support required usage flags.");
  }

  // Transforms are used before image is presented, such as rotating 90 degrees
  // on Android or something else. If identity transform is supported we use
  // it. Otherwise we just use the current one.
  VkSurfaceTransformFlagBitsKHR pre_transform = surface_capabilities.currentTransform;
  if (pre_transform & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
    pre_transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
  }

  // There are some options for presentation mode (how images are displayed):
  // MAILBOX: single entry queue (mailbox) for presentation. Removes entry at
  // vertical sync if queue not empty. When an image is presented, it replaces
  // the previous image in the mailbox, skipping a previously generated image
  // if we were fast enough. This is the lowest latency non-tearing mode of
  // presentation and is preferred.
  // IMMEDIATE: no v-sync and will screen-tear if frame is late.
  // FIFO_RELAXED: keeps queue. Will v-sync, but screen-tear if frame is late.
  // FIFO: Like above, but no tear. Only mode required by spec to be supported.
  vector<VkPresentModeKHR> present_modes =
    device->physical_device()->supported_present_modes(
      surface_.lock()->surface());

  // Don't know if bug with FIFO but it stutters despite clear 60 fps. At least
  // with immediate mode presentation is smooth.
  VkPresentModeKHR present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
  for (auto p : present_modes) {
    if (p == VK_PRESENT_MODE_MAILBOX_KHR) {
      present_mode = p;
      break;
    }
  }

  VkSwapchainCreateInfoKHR swapchain_info = {};
  swapchain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swapchain_info.pNext = nullptr;
  swapchain_info.flags = 0;
  swapchain_info.surface = surface_.lock()->surface();
  swapchain_info.minImageCount = desired_image_count;
  swapchain_info.imageFormat = color_format;
  swapchain_info.imageColorSpace = color_space;
  swapchain_info.imageExtent = surface_resolution;
  // Number of layers in images. Normally 1, but could be higher if we create
  // multiview or stereoscopic images.
  swapchain_info.imageArrayLayers = 1;
  swapchain_info.imageUsage = usage_flags;
  // How images will be used across different queue families. Exclusive means
  // that the image can only be used by one queue family at a time; if another
  // queue family will use the image, ownership must be explicitly transferred.
  // Exclusive has best performance. Concurrent on the other hand means that
  // the image may be used by multiple queue families without transferring
  // ownership. In that case one must also specify |queueFamilyIndexCount| and
  // |pQueueFamilyIndices| here.
  swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  // Used with concurrent sharing mode: the number of different queue families
  // from which the swapchain images will be referenced.
  //swapchain_info.queueFamilyIndexCount;
  // Used with concurrent sharing mode: array of the indices of the queue
  // families that will reference swapchain images.
  //swapchain_info.pQueueFamilyIndices;
  swapchain_info.preTransform = pre_transform;
  // How to composite with other surfaces on some windowing systems. Must be
  // available in capabilities, but seems opaque is always supported.
  swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  swapchain_info.presentMode = present_mode;
  // VK_TRUE: discard operations affecting regions of the surface that are not
  // visible. This is recommended if we will not read back presentable images
  // and if we don't depend on pixel shader side effects (we don't because we
  // render to an offscreen image and then copy to presentable image).
  swapchain_info.clipped = VK_TRUE;
  // Old swapchain that will be replaced with the new one. I think this is used
  // to create a new swapchain while we are still finishing the last rendering
  // commands to the old one.
  swapchain_info.oldSwapchain = swapchain_;

  VkSwapchainKHR new_swapchain = VK_NULL_HANDLE;
  VkResult result = device->vkCreateSwapchainKHR(&swapchain_info, nullptr, &new_swapchain);

  if (swapchain_) {
    device->vkDestroySwapchainKHR(swapchain_, nullptr);
    swapchain_ = VK_NULL_HANDLE;
  }

  if (result != VK_SUCCESS) {
    throw runtime_error("Failed to create swapchain.");
  }

  swapchain_ = new_swapchain;

  uint32_t image_count = 0;
  device->vkGetSwapchainImagesKHR(swapchain_, &image_count, nullptr);
  images_.resize(image_count);
  device->vkGetSwapchainImagesKHR(swapchain_, &image_count, images_.data());

  // Create views for images.
  image_views_.resize(image_count);

  VkImageViewCreateInfo view_info = {};
  view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
  view_info.format = color_format;
  view_info.components = {
    VK_COMPONENT_SWIZZLE_R,
    VK_COMPONENT_SWIZZLE_G,
    VK_COMPONENT_SWIZZLE_B,
    VK_COMPONENT_SWIZZLE_A
  };
  view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  view_info.subresourceRange.baseMipLevel = 0;
  view_info.subresourceRange.levelCount = 1;
  view_info.subresourceRange.baseArrayLayer = 0;
  view_info.subresourceRange.layerCount = 1;

  for (uint32_t i = 0; i < image_count; ++i) {
    view_info.image = images_[i];
    result = device->vkCreateImageView(&view_info, nullptr, &image_views_[i]);
    if (result != VK_SUCCESS) {
      throw runtime_error("Failed to create swapchain image view.");
    }
  }
}

}
