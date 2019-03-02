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
            audio->m_filename, transform->GetPosition(), UniEngine::GetInstance()->AudioManager()->VolumeTodB(90.f));

        std::cout << "Playing audio: " << audio->m_filename << " at " << glm::to_string<glm::vec3>(transform->GetPosition()) << std::endl;

      });
}

void AudioSystem::tick(ECS::World* world, float deltaTime) {
  
  auto mat = UniEngine::GetInstance()->SceneManager()->CurrentCamera()->matrices.view;

  glm::vec3 pos = mat[3];
  glm::vec3 up = mat[0];
  glm::vec3 forward = mat[2];

  UniEngine::GetInstance()->AudioManager()->Set3dListenerAndOrientation(pos, up, forward);

  std::cout << "Setting audio listener at " << glm::to_string<glm::vec3>(pos) << std::endl;

  UniEngine::GetInstance()->AudioManager()->Update();


}
