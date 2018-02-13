#include "renderer.h"

#include <algorithm>
#include <vector>
#include <vulkan-helpers/instance.h>
#include <vulkan-helpers/vk_dispatch_tables.h>

Renderer::Renderer() {
  vk_globals_.reset(new vkgen::GlobalFunctions("vulkan-1.dll"));

  create_instance();
}

Renderer::~Renderer() {
  instance_->vkDestroyInstance(nullptr);
}

void Renderer::create_instance() {
  VkApplicationInfo app_info {};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pNext = nullptr;
  app_info.apiVersion = 0;
  app_info.pApplicationName = nullptr;
  app_info.applicationVersion = 0;
  app_info.pEngineName = nullptr;
  app_info.engineVersion = 0;

  uint32_t layer_count = 0;
  vk_globals_->vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
  std::vector<VkLayerProperties> available_layers(layer_count);
  vk_globals_->vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

  std::vector<char const*> layers {
    "VK_LAYER_LUNARG_standard_validation"
  };

  for (auto l : layers) {
    if (available_layers.end() == std::find_if(available_layers.begin(), available_layers.end(), [&l](VkLayerProperties const& layer) {
      return strcmp(layer.layerName, l) == 0;
    })) {
      throw std::runtime_error(std::string("Vulkan instance layer `") + l + "` is not available.");
    }
  }

  uint32_t extension_count = 0;
  vk_globals_->vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
  std::vector<VkExtensionProperties> available_extensions(extension_count);
  vk_globals_->vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, available_extensions.data());

  std::vector<char const*> extensions {
    "VK_EXT_debug_report",
    "VK_KHR_surface",
#if defined(WIN32)
    "VK_KHR_win32_surface"
#endif
  };

  auto extension_it = extensions.begin();
  for (auto e : extensions) {
    if (available_extensions.end() == std::find_if(available_extensions.begin(), available_extensions.end(), [&e](VkExtensionProperties const& ext) {
      return strcmp(ext.extensionName, e) == 0;
    })) {
      throw std::runtime_error(std::string("Vulkan instance extension `") + e + "` is not available.");
    }
  }

  VkInstanceCreateInfo instance_info {};
  instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instance_info.pNext = nullptr;
  instance_info.flags = 0;
  instance_info.pApplicationInfo = &app_info;
  instance_info.enabledLayerCount = layers.size();
  instance_info.ppEnabledLayerNames = layers.data();
  instance_info.enabledExtensionCount = extensions.size();
  instance_info.ppEnabledExtensionNames = extensions.data();

  VkInstance instance { VK_NULL_HANDLE };
  VkResult result = vk_globals_->vkCreateInstance(&instance_info, nullptr, &instance);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Could not create Vulkan instance.");
  }

  instance_.reset(new vk::Instance(instance, vk_globals_.get()));
}
