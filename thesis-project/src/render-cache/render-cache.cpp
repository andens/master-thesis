#include "render-cache.h"

#include <algorithm>

void RenderCache::start_rendering(uint32_t job, RenderObject object_type) {
  jobs_[job] = object_type;
}

void RenderCache::stop_rendering(uint32_t job) {
  jobs_.erase(job);
}

void RenderCache::enumerate(std::function<void(uint32_t job, RenderObject object_type)> const& it) {
  std::for_each(jobs_.begin(), jobs_.end(), [&it](std::pair<uint32_t, RenderObject> const& pair) {
    it(pair.first, pair.second);
  });
}
