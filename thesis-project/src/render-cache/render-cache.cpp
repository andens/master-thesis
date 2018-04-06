#include "render-cache.h"

#include <algorithm>

void RenderCache::start_rendering(uint32_t job, RenderObject object_type, Pipeline pipeline) {
  // Note: this is a very trivial approach to adding a render job. We do not
  // consider whether the job is already in the cache. It would be proper to
  // account for this and perhaps result in a no-op or change in geometry.
  JobContext context;
  context.job = job;
  context.object_type = object_type;
  context.pipeline = pipeline;
  context.user_data = nullptr;
  context.was_enumerated_as_change = false;
  changes_.push_back(std::make_pair(Change::Add, context));
}

void RenderCache::stop_rendering(uint32_t job) {
  auto existing = jobs_.find(job);
  if (existing != jobs_.end()) {
    changes_.push_back(std::make_pair(Change::Remove, std::move(existing->second)));
    jobs_.erase(existing);
  }
}

void RenderCache::dirtify(uint32_t job) {
  auto existing = jobs_.find(job);
  if (existing != jobs_.end()) {
    changes_.push_back(std::make_pair(Change::Modify, existing->second));
  }
}

void RenderCache::enumerate_all(std::function<void*(JobContext const&)> const& it) {
  enumerate_changes([&it](Change, JobContext const& context) -> void* {
    return it(context);
  });

  std::for_each(jobs_.begin(), jobs_.end(), [&it](std::pair<const uint32_t, JobContext>& context) {
    // Only process jobs that were not enumerated as a change, resetting those
    // that were.
    if (context.second.was_enumerated_as_change) {
      context.second.was_enumerated_as_change = false;
    }
    else {
      auto user_data = it(context.second);
      context.second.user_data = user_data;
    }
  });
}

void RenderCache::enumerate_changes(std::function<void*(Change change, JobContext const&)> const& it) {
  std::for_each(changes_.begin(), changes_.end(), [this, &it](std::pair<Change, JobContext>& c) {
    auto user_data = it(c.first, c.second);
    c.second.user_data = user_data;
    c.second.was_enumerated_as_change = true;

    // When a job has been added or modified we synchronize with the complete
    // job store. Removed jobs have already been removed from the job store and
    // are just included in changes for informative purposes.
    if (c.first == Change::Add || c.first == Change::Modify) {
      jobs_[c.second.job] = std::move(c.second);
    }
  });

  changes_.clear();
}

size_t RenderCache::job_count() const {
  return jobs_.size();
}

void RenderCache::clear() {
  jobs_.clear();
  changes_.clear();
}
