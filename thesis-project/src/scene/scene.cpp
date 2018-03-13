#include "scene.h"

#include "../renderer/renderer.h"
#include "../render-cache/render-cache.h"

void Scene::update(float delta_time, Renderer& renderer) {
  // Simple logic to stop and start rendering the first object in intervals
  // of one second. Every new reappearance switches mesh to demonstrate that
  // draw calls can not only be individually updated, they can also completely
  // alter what is rendered.
  static float time = 0.0f;
  static bool stop_rendering = false;
  static bool box = true;
  time += delta_time;
  if (time > 1.0f) {
    time = 0.0f;
    stop_rendering = !stop_rendering;
    renderer.borrow_render_cache([](RenderCache& cache) {
      if (stop_rendering) {
        cache.stop_rendering(0);
      }
      else {
        box = !box;
        cache.start_rendering(0, box ? RenderObject::Box : RenderObject::Sphere);
      }
    });
  }
}

Scene::Scene(Renderer& renderer) {
  renderer.update_transform(0, DirectX::XMMatrixTranslation(0.0f, 0.0f, 10.0f));
  renderer.update_transform(1, DirectX::XMMatrixTranslation(5.0f, 0.0f, 10.0f));

  renderer.borrow_render_cache([](RenderCache& cache) {
    cache.start_rendering(0, RenderObject::Box);
    cache.start_rendering(1, RenderObject::Sphere);
  });
}
