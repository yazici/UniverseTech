#include "AudioSystem.h"
#include "../AudioEngine.h"
#include "../UniEngine.h"
#include "../SceneRenderer.h"
#include "../SceneManager.h"

void AudioSystem::receive(ECS::World* world, const LevelStartEvent& event) {
  auto engine = UniEngine::GetInstance();
  auto renderer = engine->GetSceneRenderer();

  world->each<AudioComponent, TransformComponent>(
    [&](ECS::Entity * ent, ECS::ComponentHandle<AudioComponent> audio,
      ECS::ComponentHandle<TransformComponent> transform) {

        if (audio->m_isPlaying) {
          audio->m_channel = UniEngine::GetInstance()->GetAudioManager()->PlaySoundFile(
            audio->m_filename, transform->GetPosition(), audio->m_volume);

          std::cout << "Playing audio: " << audio->m_filename << " at " << glm::to_string<glm::vec3>(transform->GetPosition()) << std::endl;
        }

    });
}

void AudioSystem::receive(ECS::World* world, const ECS::Events::OnComponentRemoved<AudioComponent>& event) {
  auto engine = UniEngine::GetInstance();
  auto renderer = engine->GetSceneRenderer();

  auto audio = event.component;
  if (audio->m_isPlaying) {
    //std::cout << "Stopping audio channel " << audio->m_channel << std::endl;
    UniEngine::GetInstance()->GetAudioManager()->StopChannel(audio->m_channel);
    //std::cout << "Stopped audio channel " << audio->m_channel << std::endl;
  }

}


void AudioSystem::tick(ECS::World* world, float deltaTime) {

  auto cam = UniEngine::GetInstance()->GetSceneManager()->CurrentScene()->GetCameraObject();
  auto transform = cam->GetComponent<TransformComponent>();
  auto physics = cam->GetComponent<PhysicsComponent>();

  glm::vec3 pos = transform->GetPosition();
  glm::vec3 up = transform->TransformLocalDirectionToWorldSpace({ 0, -1, 0 });
  glm::vec3 forward = transform->TransformLocalDirectionToWorldSpace({ 0, 0, 1 });
  glm::vec3 velocity = physics->m_Velocity;

  UniEngine::GetInstance()->GetAudioManager()->Set3dListenerAndOrientation(pos, up, forward, velocity);

  //std::cout << "Setting audio listener at " << glm::to_string<glm::vec3>(pos) << std::endl;

  UniEngine::GetInstance()->GetAudioManager()->Update();


}
