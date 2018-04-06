#pragma once

#include <cstdint>
#include <functional>
#include <unordered_map>
#include <vector>
#include "../render-object-type/render-object-type.h"

// Purpose: track changes to the set of render jobs. Only tracks what is to be
// rendered. Creating the render jobs themselves along with vertex allocations
// and other accompanying resources belong somewhere else (i.e., renderer).
// Note: For ease of implementation, jobs correspond to pre-allocated slots
// so that I can focus on incremental changes, but not to which shader data
// they are mapped.
class RenderCache {
public:
  enum class Pipeline {
    Alpha,
    Beta,
  };

  struct JobContext {
    uint32_t job { ~0u };
    RenderObject object_type;
    Pipeline pipeline;
    void* user_data { nullptr };
    bool was_enumerated_as_change { false };
  };

  enum class Change {
    Add,
    Remove,
    Modify
  };

public:
  void start_rendering(uint32_t job, RenderObject object_type, Pipeline pipeline);
  void stop_rendering(uint32_t job);
  void dirtify(uint32_t job);

  void enumerate_all(std::function<void*(JobContext const&)> const& it);
  void enumerate_changes(std::function<void*(Change change, JobContext const&)> const& it);
  size_t job_count() const;

private:
  std::unordered_map<uint32_t, JobContext> jobs_;
  std::vector<std::pair<Change, JobContext>> changes_;
};
