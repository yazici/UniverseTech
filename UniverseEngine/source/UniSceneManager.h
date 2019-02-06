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

 public:
  UniSceneManager() = default;
  ~UniSceneManager() = default;

  std::shared_ptr<UniScene> m_CurrentScene;

  void Initialise();
  void LoadScene(std::string sceneName);
  void UnloadCurrentScene();
  void LoadAssets(std::string sceneName);
  void RequestNewScene(std::string sceneName);
  void Shutdown();
  std::shared_ptr<UniScene> CurrentScene();
  void Tick(float frameTimer);
  bool CheckNewScene();
};
