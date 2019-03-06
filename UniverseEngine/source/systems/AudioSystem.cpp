#include "AudioSystem.h"
#include "../UniAudioEngine.h"
#include "../UniEngine.h"
#include "../UniSceneRenderer.h"
#include "../UniSceneManager.h"

void AudioSystem::receive(ECS::World* world, const LevelStartEvent& event) {
  auto engine = UniEngine::GetInstance();
  auto renderer = engine->SceneRenderer();

  world->each<AudioComponent, TransformComponent>(
      [&](ECS::Entity* ent, ECS::ComponentHandle<AudioComponent> audio,
          ECS::ComponentHandle<TransformComponent> transform) {
        UniEngine::GetInstance()->AudioManager()->PlaySoundFile(
            audio->m_filename, transform->GetPosition(), audio->m_volume);

        std::cout << "Playing audio: " << audio->m_filename << " at " << glm::to_string<glm::vec3>(transform->GetPosition()) << std::endl;

      });
}

void AudioSystem::tick(ECS::World* world, float deltaTime) {
  
  auto cam = UniEngine::GetInstance()->SceneManager()->CurrentScene()->GetCameraObject();
  auto transform = cam->GetComponent<TransformComponent>();
  auto physics = cam->GetComponent<PhysicsComponent>();

  glm::vec3 pos = transform->GetPosition();
  glm::vec3 up = transform->TransformLocalDirectionToWorldSpace({ 0, -1, 0 });
  glm::vec3 forward = transform->TransformLocalDirectionToWorldSpace({ 0, 0, 1 });
  glm::vec3 velocity = physics->m_Velocity;

  UniEngine::GetInstance()->AudioManager()->Set3dListenerAndOrientation(pos, up, forward, velocity);

  //std::cout << "Setting audio listener at " << glm::to_string<glm::vec3>(pos) << std::endl;

  UniEngine::GetInstance()->AudioManager()->Update();


}
