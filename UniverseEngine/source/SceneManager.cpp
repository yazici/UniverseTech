#include "SceneManager.h"
#include "SceneRenderer.h"
#include "UniEngine.h"
#include "systems/Systems.h"

std::vector<std::string> scenelist = {"testlevel2", "testlevel"};

void SceneManager::Initialise() {
  std::cout << "Initialising new scene." << std::endl; 
}

void SceneManager::LoadScene(std::string sceneName) {
  std::cout << "Loading assets for scene: " << sceneName << std::endl;

  if (m_renderers.find(sceneName) == m_renderers.end()) {
    std::cout << "%%%% CREATING SCENERENDERER FOR " << sceneName << " %%%%" << std::endl;
    auto renderer = std::make_shared<SceneRenderer>(sceneName);
    m_renderers.insert({ sceneName, renderer });
  }

  {
    auto scene = std::make_shared<Scene>(sceneName);
    scene->Initialize();
    m_scenes.insert({ sceneName, scene});
  }

  if (m_currentScene.empty()) {
    m_currentScene = sceneName;
  }

  LoadAssets(sceneName);
  std::cout << "Finished loading new scene." << std::endl;
  
}

void SceneManager::ActivateScene(std::string sceneName) {
  if (m_scenes.find(sceneName) == m_scenes.end()) {
    throw std::runtime_error("Cannot activate scene " + sceneName);
  }
  m_currentScene = sceneName;

  std::cout << "***** SCENERENDERER INIT " << sceneName << " *****" << std::endl;
  GetSceneRenderer(sceneName)->Initialise();

  EmitEvent<LevelStartEvent>({ true });

  m_UpdateScene = false;
  m_NextScene = "";

}

void SceneManager::UnloadScene(std::string sceneName) {
  std::cout << "Unloading scene " << sceneName << std::endl;
  m_scenes.at(sceneName)->Unload();
  m_scenes.erase(sceneName);

  std::cout << "***** SCENERENDERER SHUTDOWN " << sceneName << " *****" << std::endl;
  m_renderers.at(sceneName)->ShutDown();
  m_renderers.erase(sceneName);
}

void SceneManager::LoadAssets(std::string sceneName) {
  m_scenes.at(sceneName)->Load(UniEngine::GetInstance()->getAssetPath() + "levels/" + sceneName + ".json");
}

void SceneManager::RequestNewScene(std::string sceneName) {
  m_UpdateScene = true;
  m_NextScene = sceneName;
}

void SceneManager::CycleScenes() {
  m_CurrentSceneIdx++;
  if (m_CurrentSceneIdx >= scenelist.size())
    m_CurrentSceneIdx = 0;

  m_NextScene = scenelist.at(m_CurrentSceneIdx);
  m_UpdateScene = true;
}

void SceneManager::Shutdown() {
  std::vector<std::string> scenes = {};
  for (auto& [name, scene] : m_scenes) {
    scenes.push_back(name);
  }

  for (const auto& name : scenes) {

    UnloadScene(name);
  }
}

std::shared_ptr<Scene> SceneManager::CurrentScene() {
  return m_scenes.at(m_currentScene);
}

void SceneManager::Tick(float frameTimer) {
  CurrentScene()->Tick(frameTimer);
  GetSceneRenderer()->Tick(frameTimer);

}

bool SceneManager::CheckNewScene() {
  if (m_UpdateScene && !m_NextScene.empty()) {
    auto lastScene = m_currentScene;
    LoadScene(m_NextScene);
    ActivateScene(m_NextScene);
    UnloadScene(lastScene);
    return true;
  }

  return false;
}

std::shared_ptr<SceneRenderer> SceneManager::GetSceneRenderer()
{
  return GetSceneRenderer(m_currentScene);
}

std::shared_ptr<SceneRenderer> SceneManager::GetSceneRenderer(std::string sceneName)
{
  return m_renderers.at(sceneName);
}
