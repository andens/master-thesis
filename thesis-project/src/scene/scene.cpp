#include "scene.h"

#include "../renderer/renderer.h"
#include "../render-cache/render-cache.h"

void Scene::update(float delta_time, Renderer& renderer) {
}

Scene::Scene(Renderer& renderer) {
  renderer.update_transform(0, DirectX::XMMatrixTranslation(0.0f, 0.0f, 10.0f));
  renderer.update_transform(1, DirectX::XMMatrixTranslation(5.0f, 0.0f, 10.0f));

  renderer.borrow_render_cache([](RenderCache& cache) {
    cache.start_rendering(0, RenderObject::Box);
    cache.start_rendering(1, RenderObject::Sphere);
  });
}
