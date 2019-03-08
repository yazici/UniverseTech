#pragma once

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "UniScene.h"
 
class UniSceneRenderer;

class UniSceneManager {
 private:
  bool m_UpdateScene = false;
  std::string m_NextScene = "";

  size_t m_CurrentSceneIdx = 0;

  std::string m_currentScene = "";

  std::map<std::string, std::shared_ptr<UniSceneRenderer>> m_renderers;

 public:
  UniSceneManager() = default;
  ~UniSceneManager() = default;

  std::map<std::string, std::shared_ptr<UniScene>> m_scenes;

  void Initialise();
  void LoadScene(std::string sceneName);
  void ActivateScene(std::string sceneName);
  void UnloadScene(std::string sceneName);
  void LoadAssets(std::string sceneName);
  void RequestNewScene(std::string sceneName);
  void CycleScenes();
  void Shutdown();
  std::shared_ptr<UniScene> CurrentScene();
  void Tick(float frameTimer);
  bool CheckNewScene();

  std::shared_ptr<UniSceneRenderer> SceneRenderer();

  std::shared_ptr<UniSceneRenderer> SceneRenderer(std::string sceneName);
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
