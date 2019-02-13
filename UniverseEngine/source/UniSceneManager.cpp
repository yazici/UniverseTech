#include "UniSceneManager.h"
#include "UniSceneRenderer.h"
#include "UniEngine.h"

std::vector<std::string> scenelist = {"testlevel", "testlevel2"};

void UniSceneManager::Initialise() {
  std::cout << "Initialising new scene." << std::endl;
  m_CurrentScene = std::make_shared<UniScene>();
  m_CurrentScene->Initialize();

  m_renderer = UniEngine::GetInstance().SceneRenderer();

}

void UniSceneManager::LoadScene(std::string sceneName) {
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

void UniSceneManager::CycleScenes() {
  m_CurrentSceneIdx++;
  if (m_CurrentSceneIdx >= scenelist.size())
    m_CurrentSceneIdx = 0;

  m_NextScene = scenelist.at(m_CurrentSceneIdx);
  m_UpdateScene = true;

}


void UniSceneManager::Shutdown() {
  UnloadCurrentScene();
}

std::shared_ptr<UniScene> UniSceneManager::CurrentScene() {
  return m_CurrentScene;
}

void UniSceneManager::Tick(float frameTimer) {
  CurrentScene()->Tick(frameTimer);
  m_renderer->Tick(frameTimer);
  
}


bool UniSceneManager::CheckNewScene() {
  if (m_UpdateScene && !m_NextScene.empty()) {
    UnloadCurrentScene();
    Initialise();
    LoadScene(m_NextScene);
    return true;
  }

  return false;
}
