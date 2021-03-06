#include "render-cache.h"

#include <algorithm>

extern const uint32_t g_draw_call_count;

/*
void RenderCache::start_rendering(uint32_t job, RenderObject object_type, Pipeline pipeline) {
  // Note: this is a very trivial approach to adding a render job. We do not
  // consider whether the job is already in the cache. It would be proper to
  // account for this and perhaps result in a no-op or change in geometry.
  JobContext context;
  context.job = job;
  context.object_type = object_type;
  context.pipeline = pipeline;
  context.change = Change::Add;
  context.user_data = nullptr;
  context.was_enumerated_as_change = false;
  jobs_[job] = context;
}

void RenderCache::stop_rendering(uint32_t job) {
  jobs_[job].change = Change::Remove;
}
*/

void RenderCache::modify_pipeline(uint32_t job, Pipeline pipeline) {
  auto& j = jobs_[job];
  if (j.pipeline != pipeline) {
    j.change = Change::Modify;
    j.pipeline = pipeline;
  }
}

void RenderCache::dirtify(uint32_t job) {
  jobs_[job].change = Change::Modify;
}

void RenderCache::dirtify_all() {
  for (uint32_t i = 0; i < g_draw_call_count; ++i) {
    dirtify(i);
  }
}

void RenderCache::enumerate_all(std::function<void*(JobContext const&)> const& it) {
  std::for_each(jobs_.begin(), jobs_.end(), [&it](JobContext& context) {
    auto user_data = it(context);
    context.user_data = user_data;
    context.change = Change::None;
  });
}

/*
void RenderCache::enumerate_changes(std::function<void*(Change change, JobContext const&)> const& it) {
  std::for_each(jobs_.begin(), jobs_.end(), [this, &it](JobContext& c) {
    if (c.change != Change::None) {
      auto user_data = it(c.change, c);
      c.user_data = user_data;
      c.was_enumerated_as_change = true;
      c.change = Change::None;
    }
  });
}
*/

void RenderCache::clear() {
  jobs_.clear();

  for (uint32_t i = 0; i < g_draw_call_count; ++i) {
    JobContext context;
    context.job = i;
    context.object_type = i % 2 == 0 ? RenderObject::Sphere : RenderObject::Box;
    context.pipeline = Pipeline::Alpha;
    context.change = Change::Modify; // To write indirect buffers on first access
    context.user_data = nullptr;
    context.was_enumerated_as_change = false;
    jobs_.push_back(context);
  }
}

RenderCache::RenderCache() {
  jobs_.reserve(g_draw_call_count);
  clear();
}
