#include "AudioSystem.h"
#include "../UniAudioEngine.h"
#include "../UniEngine.h"

void AudioSystem::receive(ECS::World* world, const LevelStartEvent& event) {
  auto engine = UniEngine::GetInstance();
  auto renderer = engine->SceneRenderer();

  world->each<AudioComponent, TransformComponent>(
      [&](ECS::Entity* ent, ECS::ComponentHandle<AudioComponent> audio,
          ECS::ComponentHandle<TransformComponent> transform) {
        UniEngine::GetInstance()->AudioManager()->PlaySoundFile(
            audio->m_filename, transform->GetPosition());
      });
}

void AudioSystem::tick(ECS::World* world, float deltaTime) {
  UniEngine::GetInstance()->AudioManager()->Update();
}
