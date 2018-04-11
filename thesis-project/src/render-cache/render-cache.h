#pragma once

#include <cstdint>
#include <functional>
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

  enum class Change {
    Add,
    Remove,
    Modify,
    None,
  };

  struct JobContext {
    uint32_t job { ~0u };
    RenderObject object_type;
    Pipeline pipeline;
    Change change;
    void* user_data { nullptr };
    bool was_enumerated_as_change { false };
  };

public:
  // Should not be used. Had a purpose with early ideas, but for simplicity I
  // just hardcode the number of objects and they don't change.
  //void start_rendering(uint32_t job, RenderObject object_type, Pipeline pipeline); 
  //void stop_rendering(uint32_t job);

  void modify_pipeline(uint32_t job, Pipeline pipeline);
  void dirtify(uint32_t job);
  void dirtify_all();

  // Enumerate all jobs, calling the user-provided |it| function on them.
  // Changes are consumes in the process.
  void enumerate_all(std::function<void*(JobContext const&)> const& it);

  // Enumerate only jobs that have changed in some way, consuming the change in
  // the process. Note that this still iterates the entire vector; however,
  // |it| is only called for changed jobs. This is deliberate to make all
  // rendering strategies use the same traversal to eliminate variations in
  // traversal overhead. The overhead is still there, but cancels when looking
  // at differences between strategies.
  //void enumerate_changes(std::function<void*(Change change, JobContext const&)> const& it);

  void clear();

  RenderCache();

private:
  const uint32_t max_draw_calls_ { 100000 };
  std::vector<JobContext> jobs_;
};
