#pragma once

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Scene.h"


namespace uni {
  namespace render {
    class SceneRenderer;
  }

  namespace scene
  {
    class SceneManager {
    private:
      bool m_UpdateScene = false;
      std::string m_NextScene = "";

      size_t m_CurrentSceneIdx = 0;

      std::string m_currentScene = "";

      std::map<std::string, std::shared_ptr<uni::render::SceneRenderer>> m_renderers;

    public:
      SceneManager() = default;
      ~SceneManager() = default;

      std::map<std::string, std::shared_ptr<Scene>> m_scenes;

      void Initialise();
      void LoadScene(std::string sceneName);
      void ActivateScene(std::string sceneName);
      void UnloadScene(std::string sceneName);
      void LoadAssets(std::string sceneName);
      void RequestNewScene(std::string sceneName);
      void CycleScenes();
      void Shutdown();
      std::shared_ptr<Scene> CurrentScene();
      void Tick(float frameTimer);
      bool CheckNewScene();

      std::shared_ptr<uni::render::SceneRenderer> GetSceneRenderer();

      std::shared_ptr<uni::render::SceneRenderer> GetSceneRenderer(std::string sceneName);
      ECS::ComponentHandle<CameraComponent> CurrentCamera() {
        return CurrentScene()->GetCameraComponent();
      }

      template <typename T>
      void EmitEvent(const T& event);
    };

    template <typename T>
    void SceneManager::EmitEvent(const T& event) {
      CurrentScene()->m_World->emit<T>(event);
    }
  }
}
