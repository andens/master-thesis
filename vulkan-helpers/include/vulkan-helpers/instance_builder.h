#pragma once

#include <memory>
#include <vector>
#include "vulkan_include.inl"

namespace vkgen {
class GlobalFunctions;
}

namespace vk {

class Instance;

class InstanceBuilder {
public:
  void use_layer(char const* layer);
  void use_extension(char const* ext);
  std::unique_ptr<Instance> build(vkgen::GlobalFunctions* vkg);

private:
  bool layer_available(char const* layer, std::vector<VkLayerProperties> const& available);
  bool extension_available(char const* ext, std::vector<VkExtensionProperties> const& available);

private:
  std::vector<char const*> layers_;
  std::vector<char const*> extensions_;
};

}
