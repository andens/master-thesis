#include "render-cache.h"

#include <algorithm>

void RenderCache::start_rendering(uint32_t job, RenderObject object_type) {
  // Note: this is a very trivial approach to adding a render job. We do not
  // consider whether the job is already in the cache. It would be proper to
  // account for this and perhaps result in a no-op or change in geometry.
  JobContext context;
  context.job = job;
  context.object_type = object_type;
  context.user_data = nullptr;
  changes_.push_back(std::make_pair(Change::Add, context));
}

void RenderCache::stop_rendering(uint32_t job) {
  auto existing = jobs_.find(job);
  if (existing != jobs_.end()) {
    changes_.push_back(std::make_pair(Change::Remove, std::move(existing->second)));
    jobs_.erase(existing);
  }
}

void RenderCache::enumerate_all(std::function<void*(JobContext const&)> const& it) {
  std::for_each(jobs_.begin(), jobs_.end(), [&it](std::pair<const uint32_t, JobContext>& context) {
    auto user_data = it(context.second);
    context.second.user_data = user_data;
  });

  enumerate_changes([&it](Change, JobContext const& context) -> void* {
    return it(context);
  });
}

// Note: Only added elements for now
void RenderCache::enumerate_changes(std::function<void*(Change change, JobContext const&)> const& it) {
  std::for_each(changes_.begin(), changes_.end(), [this, &it](std::pair<Change, JobContext>& c) {
    auto user_data = it(c.first, c.second);
    c.second.user_data = user_data;

    // Depending on what kind of change we are doing, it might not be as simple
    // as just updating (or inserting) a value.
    jobs_[c.second.job] = std::move(c.second);
  });

  changes_.clear();
}

size_t RenderCache::job_count() const {
  return jobs_.size();
}
