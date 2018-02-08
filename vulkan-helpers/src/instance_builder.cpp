#include "instance_builder.h"

#include "instance.h"

using namespace std;

namespace vk {

void InstanceBuilder::use_layer(char const* layer) {
  layers_.push_back(layer);
}

void InstanceBuilder::use_extension(char const* ext) {
  extensions_.push_back(ext);
}

unique_ptr<Instance> InstanceBuilder::build(vkgen::GlobalFunctions* vkg) {
  // Find out what validation layers the loader knows about. We can get layers
  // from the SDK or the loader and validation layers Khronos repo on Github.
  // However, layers can also be acquired from other places. For example, I
  // already had steam overlay layer installed.
  uint32_t layer_count = 0;
  vkg->vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
  vector<VkLayerProperties> available_layers(layer_count);
  vkg->vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

  // If any layer is unavailable, instance creation fails.
  for (auto layer : layers_) {
    if (!layer_available(layer, available_layers)) {
      throw runtime_error(string("Instance layer `") + layer + string("` is unavailable."));
    }
  }

  // Find out what extensions are available. Extensions expose additional
  // functionality not required by the core spec. There are also device level
  // extensions.
  uint32_t extension_count = 0;
  vkg->vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
  vector<VkExtensionProperties> available_extensions(extension_count);
  vkg->vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, available_extensions.data());

  // If any extension is unavailable, instance creation will fail.
  for (auto ext : extensions_) {
    if (!extension_available(ext, available_extensions)) {
      throw runtime_error(string("Instance extension `") + ext + string("` is unavailable."));
    }
  }

  VkApplicationInfo app_info = {};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.apiVersion = VK_MAKE_VERSION(1, 0, 0);

  VkInstanceCreateInfo instance_info = {};
  instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instance_info.pApplicationInfo = &app_info;
  instance_info.enabledLayerCount = static_cast<uint32_t>(layers_.size());
  instance_info.ppEnabledLayerNames = layers_.data();
  instance_info.enabledExtensionCount = static_cast<uint32_t>(extensions_.size());
  instance_info.ppEnabledExtensionNames = extensions_.data();

  // The Vulkan instance stores application-specific data as there is no global
  // state in Vulkan.
  VkInstance instance = VK_NULL_HANDLE;
  VkResult result = vkg->vkCreateInstance(&instance_info, nullptr, &instance);
  if (result != VK_SUCCESS) {
    throw runtime_error("Could not create Vulkan instance.");
  }

  return unique_ptr<vk::Instance>(new vk::Instance(instance, vkg));
}

bool InstanceBuilder::layer_available(char const* layer, vector<VkLayerProperties> const& available) {
  for (auto& l : available) {
    if (strcmp(layer, l.layerName) == 0) {
      return true;
    }
  }
  return false;
}

bool InstanceBuilder::extension_available(char const* ext, vector<VkExtensionProperties> const& available) {
  for (auto& e : available) {
    if (strcmp(ext, e.extensionName) == 0) {
      return true;
    }
  }
  return false;
}

}
