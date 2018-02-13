#pragma once

#include <memory>

namespace vkgen {
class GlobalFunctions;
}

namespace vk {
class Instance;
}

class Renderer {
public:
  Renderer();
  ~Renderer();

private:
  void create_instance();

private:
  std::unique_ptr<vkgen::GlobalFunctions> vk_globals_;
  std::unique_ptr<vk::Instance> instance_;
};
