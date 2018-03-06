#pragma once

#include <cstdint>
#include <functional>
#include <map>
#include "../render-object-type/render-object-type.h"

// Purpose: track changes to the set of render jobs. Only tracks what is to be
// rendered. Creating the render jobs themselves along with vertex allocations
// and other accompanying resources belong somewhere else (i.e., renderer).
// Note: For ease of implementation, jobs correspond to pre-allocated slots
// so that I can focus on incremental changes, but not to which shader data
// they are mapped.
class RenderCache {
public:
  void start_rendering(uint32_t job, RenderObject object_type);
  void stop_rendering(uint32_t job);

  void enumerate(std::function<void(uint32_t job, RenderObject object_type)> const& it);

private:
  std::map<uint32_t, RenderObject> jobs_;
};
