#include "ModelRenderSystem.h"
#include "../UniEngine.h"
#include "../UniInput.h"
#include "../UniSceneManager.h"
#include "../UniSceneRenderer.h"
#include "../materials/ModelMaterial.h"

void ModelRenderSystem::receive(ECS::World* world, const RenderEvent& event) {
  auto engine = UniEngine::GetInstance();
  auto renderer = engine->SceneRenderer();

  world->each<ModelComponent>(
      [&](ECS::Entity* ent, ECS::ComponentHandle<ModelComponent> model) {
        auto materials = model->GetMaterialIDs();
        for_each(materials.begin(), materials.end(), [&](std::string matID) {
          auto material = renderer->GetMaterialByID<ModelMaterial>(matID);
          material->AddToCommandBuffer(event.cmdBuffer, model);
          /*std::cout << "rendering model " << model->GetName()
                    << " with material " << matID << std::endl;*/
        });
      });
}

void ModelRenderSystem::receive(ECS::World* world, const RemoveEvent& event) {
  event.component->Destroy();
}

void ModelRenderSystem::tick(ECS::World* world, float deltaTime) {}
