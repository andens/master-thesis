#pragma once

// This file is used by vk_dispatch_tables.h to have us set platforms.
// Instead of linking statically to the loader library, dynamic symbol
// lookup can be used to build dispatch tables. This allows graceful
// shutdown if the library is unavailable and is the fastest way to call
// Vulkan functions. Furthermore, some extension functions are not even
// available in the static library, in which case they would have to be
// loaded manually anyway. OS functions must be used to get a pointer to
// vkGetInstanceProcAddr. It's also the only function guaranteed to be
// exported; other functions can be acquired via that one.

// Disable function prototypes in vulkan.h, as we load functions dynamically.
#define VK_NO_PROTOTYPES

// Platforms for Mir and Wayland: doesn't seem to be a preprocessor define for
// these as they can be used together with X on the same system. Perhaps it's
// possible to check in runtime if it's possible to to create one or the other
// and simple define both and switch between which one was chosen. Maybe store
// window parameters in a union and interpret it based on a control variable?

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#else
#error "Unsupported OS"
#endif

#include <vulkan/vulkan.h>
