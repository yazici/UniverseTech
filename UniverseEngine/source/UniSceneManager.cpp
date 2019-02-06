#include "UniSceneManager.h"
#include "UniEngine.h"

void UniSceneManager::LoadScene(std::string sceneName) {
  std::cout << "Initialising new scene." << std::endl;
  m_CurrentScene = std::make_shared<UniScene>();
  m_CurrentScene->Initialize(m_Engine);
  std::cout << "Loading assets for scene: " << sceneName << std::endl;
  LoadAssets(sceneName);
  std::cout << "Finished loading new scene." << std::endl;

  m_UpdateScene = false;
  m_NextScene = "";
}

void UniSceneManager::UnloadCurrentScene() {
  std::cout << "Unloading current scene." << std::endl;
  m_CurrentScene->Unload();
  m_CurrentScene.reset();
}

void UniSceneManager::LoadAssets(std::string sceneName) {
  m_CurrentScene->Load(getAssetPath() + "levels/" + sceneName + ".json");
}

void UniSceneManager::RequestNewScene(std::string sceneName) {
  m_UpdateScene = true;
  m_NextScene = sceneName;
}

void UniSceneManager::Shutdown() {
  UnloadCurrentScene();
}

std::shared_ptr<UniScene> UniSceneManager::CurrentScene() {
  return m_CurrentScene;
}

void UniSceneManager::Tick(float frameTimer) {
  if (m_UpdateScene && !m_NextScene.empty()) {
    UnloadCurrentScene();
    LoadScene(m_NextScene);
    return;
  }

  CurrentScene()->Tick(frameTimer);
}
