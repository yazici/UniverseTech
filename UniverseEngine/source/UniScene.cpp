#include "UniScene.h"
#include <cmath>
#include <iosfwd>
#include <memory>
#include <nlohmann/json.hpp>
#include "Materials.h"
#include "UniEngine.h"
#include "UniAudioEngine.h"
#include "UniSceneManager.h"
#include "UniSceneRenderer.h"
#include "components/Components.h"
#include "systems/Systems.h"
#include "UniAssetManager.h"
#include "UniAsset.h"
#include <set>

using json = nlohmann::json;

UniScene::UniScene() {
  m_World = nullptr;
}

UniScene::UniScene(std::string name)
{
  m_World = nullptr;
  m_Name = name;
}

UniScene::~UniScene() {}

void UniScene::Initialize() {
  auto engine = UniEngine::GetInstance();
  m_World = ECS::World::createWorld();
  m_World->registerSystem(new MovementSystem());
  m_World->registerSystem(new CameraSystem());
  m_World->registerSystem(new PlanetRenderSystem());
  m_World->registerSystem(new PlayerControlSystem());
  m_World->registerSystem(new GravitySystem());
  m_World->registerSystem(new PhysicsSystem());
  m_World->registerSystem(new ModelRenderSystem());
  m_World->registerSystem(new AudioSystem());

  m_CurrentCamera = Make<UniSceneObject>(glm::vec3(0), "player camera");
  m_CurrentCamera->AddComponent<CameraComponent>(
    m_CurrentCamera->GetTransform(),
    (float)engine->width / (float)engine->height, 50.f, 0.01f, 10000.f);
  // m_CurrentCamera->AddComponent<MovementComponent>();
  m_CurrentCamera->AddComponent<PlayerControlComponent>();
  m_CurrentCamera->AddComponent<PhysicsComponent>(5000.0);
  m_CurrentCamera->m_Entity->get<PhysicsComponent>()->SetSceneObject(
    m_CurrentCamera);
}

void UniScene::Load(std::string filename) {

  auto assetManager = UniEngine::GetInstance()->AssetManager();
  auto renderer = UniEngine::GetInstance()->SceneManager()->SceneRenderer(m_Name);

  std::ifstream t(filename);
  std::stringstream buffer;
  buffer << t.rdbuf();

  json data = json::parse(buffer.str());
  auto level = data["level"];
  m_Name = level["name"];

  std::vector<std::string> assetStrings = {};

  for (const auto& row : level.at("sceneObjects")) {
    for (const auto& component : row.at("components")) {
      if (component.find("asset") != component.end()) {
        assetStrings.push_back(component.at("asset"));
      }
    }
  }

  assetManager->CheckImported(assetStrings);


  for (const auto& row : level.at("sceneObjects")) {
    std::string soName = row.at("name");

    glm::vec3 lpos = { 0, 0, 0 };
    if (row.find("position") != row.end()) {
      auto pos = row.at("position");
      lpos = glm::vec3(pos[0], pos[1], pos[2]);
    }

    auto sceneObject = Make<UniSceneObject>(lpos, soName);

    if (row.find("rotation") != row.end()) {
      auto rot = row.at("rotation");
      sceneObject->GetTransform()->SetRotation({ rot[0], rot[1], rot[2] });
    }

    if (row.find("enabled") != row.end()) {
      sceneObject->SetRendered(row.at("enabled"));
    }

    // iterate over components and either assign from assets or create
    // TODO - move this into the plugin system for asset importers
    for (const auto& component : row.at("components")) {

      // AUDIO
      if (component.at("type") == "audio") {
        // audio assets are not loaded in the same way as other types. the filename is
        // registered with the audio engine and just needs to be referenced for playback
        std::cout << "Assigning audio component: " << component.at("asset") << std::endl;
        auto audio = sceneObject->AddComponent<AudioComponent>(static_cast<std::string>(component.at("asset")));
        if (component.find("volume") != component.end()) {
          audio->m_volume = component.at("volume");
        }
        if (component.find("looping") != component.end()) {
          audio->m_isLooping = component.at("looping");
        }
        if (component.find("paused") != component.end()) {
          audio->m_isPlaying = !component.at("paused");
        }
      }

      // MOVEMENT
      if (component.at("type") == "movement") {
        std::cout << "Creating movement component " << std::endl;
        auto vel = glm::vec3(0);
        auto rot = glm::vec3(0);

        if (component.find("velocity") != component.end()) {
          auto velocity = component.at("velocity");
          vel = glm::vec3(velocity.at(0), velocity.at(1), velocity.at(2));
        }

        if (component.find("rotation") != component.end()) {
          auto rotation = component.at("rotation");
          rot = glm::vec3(rotation.at(0), rotation.at(1), rotation.at(2));
        }

        sceneObject->AddComponent<MovementComponent>(vel, rot);
      }

      // MODEL
      if (component.at("type") == "model") {
        std::cout << "Assigning model component: " << component.at("asset") << std::endl;

        auto asset = assetManager->GetAsset<uni::assets::UniAssetModel>(component.at("asset"));

        ECS::ComponentHandle<ModelComponent> model =
          sceneObject->AddComponent<ModelComponent>(asset->m_path);
        model->SetSceneObject(sceneObject);
        model->m_Materials = asset->m_materials;

        for (const auto& mat : asset->m_materials) {
          
          if (renderer->GetMaterialByID<ModelMaterial>(mat) == nullptr) {
            auto matAsset = assetManager->GetAsset<uni::assets::UniAssetMaterial>(mat);
            renderer->RegisterMaterial(mat, matAsset->m_material);
          }
        }

      }

      if (component.at("type") == "light") {
        std::cout << "Creating light component " << std::endl;

        auto radius = component.at("radius");
        auto colarray = component.at("color");
        auto color =
          glm::vec4(colarray[0], colarray[1], colarray[2], colarray[3]);
        auto enabled = component.at("on");
        sceneObject->AddComponent<LightComponent>(radius, color, enabled);
      }


    }
  }

  auto playerStart = level.at("playerStart");
  auto playerPos = playerStart.at("position");
  auto playerRot = playerStart.at("rotation");

  auto camObj = GetCameraObject();
  camObj->GetTransform()->SetPosition(
    glm::vec3(playerPos.at(0), playerPos.at(1), playerPos.at(2)));
  camObj->GetTransform()->SetRotation(
    { playerRot[0], playerRot[1], playerRot[2] });
  GetCameraComponent()->CalculateView(camObj->GetTransform());

  std::cout << "Scene fully loaded." << std::endl;
}

void UniScene::Unload() {
  std::cout << "Shutting down scene." << std::endl;

  m_World->destroyWorld();

  for (auto so : m_SceneObjects) {
    so.reset();
  }
  m_SceneObjects.clear();
}

void UniScene::Tick(float deltaTime) {
  m_World->tick(deltaTime);
  // m_BodyTest->Update();
}

std::vector<std::shared_ptr<UniSceneObject>> UniScene::GetRenderedObjects() {
  if (!m_RenderedObjectCache.empty()) {
    return m_RenderedObjectCache;
  }

  for_each(m_SceneObjects.begin(), m_SceneObjects.end(),
    [this](std::shared_ptr<UniSceneObject> so) {
      if (so->IsRendered()) {
        m_RenderedObjectCache.push_back(so);
      }
    });

  return m_RenderedObjectCache;
}

void UniScene::AddSceneObject(std::shared_ptr<UniSceneObject> so) {
  m_SceneObjects.push_back(so);
  m_RenderedObjectCache.clear();
}
