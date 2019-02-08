#pragma once

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "UniScene.h"

class UniSceneManager {
 private:
  bool m_UpdateScene = false;
  std::string m_NextScene = "";

  size_t m_CurrentSceneIdx = 0;

 public:
  UniSceneManager() = default;
  ~UniSceneManager() = default;

  std::shared_ptr<UniScene> m_CurrentScene;

  void Initialise();
  void LoadScene(std::string sceneName);
  void UnloadCurrentScene();
  void LoadAssets(std::string sceneName);
  void RequestNewScene(std::string sceneName);
  void CycleScenes();
  void Shutdown();
  std::shared_ptr<UniScene> CurrentScene();
  void Tick(float frameTimer);
  bool CheckNewScene();

  ECS::ComponentHandle<CameraComponent> CurrentCamera() {
    return CurrentScene()->GetCameraComponent();
  }

  template <typename T>
  void EmitEvent(const T& event);
};

template <typename T>
void UniSceneManager::EmitEvent(const T& event) {
  CurrentScene()->m_World->emit<T>(event);
}
