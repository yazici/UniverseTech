#include "UniScene.h"
#include <cmath>
#include <iosfwd>
#include <memory>
#include <nlohmann/json.hpp>
#include "UniEngine.h"
#include "UniSceneRenderer.h"
#include "Materials.h"
#include "components/Components.h"
#include "systems/Systems.h"

using json = nlohmann::json;

UniScene::UniScene() {}

UniScene::~UniScene() {
  std::cout << "Shutting down scene." << std::endl;

  for (auto so : m_SceneObjects) {
    so.reset();
  }
  m_SceneObjects.clear();
  m_World->destroyWorld();
}

void UniScene::Initialize() {
  UniEngine& engine = UniEngine::GetInstance();
  m_World = ECS::World::createWorld();
  m_World->registerSystem(new MovementSystem());
  m_World->registerSystem(new CameraSystem());
  m_World->registerSystem(new PlanetRenderSystem());
  m_World->registerSystem(new PlayerControlSystem());
  m_World->registerSystem(new GravitySystem());
  m_World->registerSystem(new PhysicsSystem());

  m_CurrentCamera = Make<UniSceneObject>(glm::vec3(0), "player camera");
  m_CurrentCamera->AddComponent<CameraComponent>(
      m_CurrentCamera->GetTransform(),
      (float)engine.width / (float)engine.height, 50.f, 0.01f, 1000.f);
  // m_CurrentCamera->AddComponent<MovementComponent>();
  m_CurrentCamera->AddComponent<PlayerControlComponent>();
  m_CurrentCamera->AddComponent<PhysicsComponent>(5000.0);
  m_CurrentCamera->m_Entity->get<PhysicsComponent>()->SetSceneObject(
      m_CurrentCamera);
}

void UniScene::Load(std::string filename) {
  UniEngine& engine = UniEngine::GetInstance();
  auto renderer = engine.SceneRenderer();

  std::ifstream t(filename);
  std::stringstream buffer;
  buffer << t.rdbuf();

  json data = json::parse(buffer.str());
  auto level = data["level"];
  m_Name = level["name"];

  std::map<std::string, std::shared_ptr<UniMaterial>> loadedMaterials;

  for (const auto& so : level.at("materials")) {
    std::string soType = so.at("type");

    if (soType == "model material") {
      std::cout << "Loading model material: " << so.at("name") << std::endl;
      // 	m_Name = jsonData["name"];
      std::string materialID = so.at("name");

      std::string texturePath = "";
      if (so.find("textureMap") != so.end())
        texturePath = so.at("textureMap");

      std::string normalPath = "";
      if (so.find("normalMap") != so.end())
        normalPath = so.at("normalMap");

      std::string metallicPath = "";
      if (so.find("metallicMap") != so.end())
        metallicPath = so.at("metallicMap");

      std::string roughnessPath = "";
      if (so.find("roughnessMap") != so.end())
        roughnessPath = so.at("roughnessMap");

      std::string specularPath = "";
      if (so.find("specularMap") != so.end())
        specularPath = so.at("specularMap");

      std::string emissivePath = "";
      if (so.find("emissiveMap") != so.end())
        emissivePath = so.at("emissiveMap");

      std::string aoPath = "";
      if (so.find("aoMap") != so.end())
        aoPath = so.at("aoMap");

      glm::vec3 baseColour = glm::vec3(1);
      if (so.find("colour") != so.end()) {
        auto bc = so.at("colour");
        baseColour = glm::vec3(bc[0], bc[1], bc[2]);
      }

      glm::vec3 emissiveColour = glm::vec3(0);
      if (so.find("emissive") != so.end()) {
        auto bc = so.at("emissive");
        emissiveColour = glm::vec3(bc[0], bc[1], bc[2]);
      }

      float specular = 0.5f;
      float metallic = 0.f;
      float roughness = 1.f;

      if (so.find("specular") != so.end()) {
        specular = so.at("specular");
      }

      if (so.find("metallic") != so.end()) {
        metallic = so.at("metallic");
      }

      if (so.find("roughness") != so.end()) {
        roughness = so.at("roughness");
      }

      short layers = 1;
      short layer = 0;

      auto material = std::make_shared<ModelMaterial>(materialID, layers);
      material->LoadTexture("texture", layer, texturePath);
      material->LoadTexture("normal", layer, normalPath);
      material->LoadTexture("metallic", layer, metallicPath);
      material->LoadTexture("specular", layer, specularPath);
      material->LoadTexture("roughness", layer, roughnessPath);
      material->LoadTexture("emissive", layer, emissivePath);
      material->LoadTexture("ao", layer, aoPath);
      material->SetBaseColour(baseColour, layer);
      material->SetEmissiveColour(emissiveColour, layer);
      material->SetRoughness(roughness, layer);
      material->SetMetallic(metallic, layer);
      material->SetSpecular(specular, layer);

      renderer->RegisterMaterial(material);

      loadedMaterials.insert({materialID, material});
    }
  }

  for (const auto& so : level.at("sceneObjects")) {
    std::string soType = so.at("type");

    if (soType == "model") {

      if (so.find("enabled") != so.end()) {
        if (so.at("enabled") == false) {
          continue;
        }
      }

      std::cout << "Loading model: " << so.at("name") << std::endl;
      // 	m_Name = jsonData["name"];
      auto modelPath = so.at("mesh");
      auto materialID = so.at("materialID");

      if (loadedMaterials.find(materialID) == loadedMaterials.end()) {
        throw std::runtime_error("Cannot load model with material id: " +
                                 materialID);
      }

      glm::vec3 mpos;
      if (so.find("position") != so.end()) {
        auto pos = so.at("position");
        mpos = glm::vec3(pos[0], pos[1], pos[2]);
      }

      std::cout << "Creating model path: " << modelPath << std::endl;
      auto model = Make<UniModel>(mpos, (std::string)so.at("name"), modelPath,
                                  materialID);

      if (so.find("rotation") != so.end()) {
        auto rot = so.at("rotation");
        model->GetTransform()->SetRotation({rot[0], rot[1], rot[2]});
      }

      glm::vec3 createScale = {so.at("createScale")[0], so.at("createScale")[1],
                               so.at("createScale")[2]};
      glm::vec2 createUVScale = {so.at("createUVScale")[0],
                                 so.at("createUVScale")[1]};
      glm::vec3 createOffset = glm::vec3(0);

      if (so.find("createOffset") != so.end()) {
        createOffset = {so.at("createOffset")[0], so.at("createOffset")[1],
                        so.at("createOffset")[2]};
      }

      if (so.find("components") != so.end()) {
        auto components = so.at("components");
        if (components.find("movement") != components.end()) {
          auto movement = components.at("movement");
          auto vel = glm::vec3(0);
          auto rot = glm::vec3(0);

          if (movement.find("velocity") != movement.end()) {
            auto velocity = movement.at("velocity");
            vel = glm::vec3(velocity.at(0), velocity.at(1), velocity.at(2));
          }

          if (movement.find("rotation") != movement.end()) {
            auto rotation = movement.at("rotation");
            rot = glm::vec3(rotation.at(0), rotation.at(1), rotation.at(2));
          }

          model->AddComponent<MovementComponent>(vel, rot);
        }
      }

      model->SetCreateInfo(createOffset, createScale, createUVScale);
      model->Load(renderer->GetVertexLayout(), engine.vulkanDevice,
                  engine.GetQueue(), true);

      if (so.find("enabled") != so.end()) {
        model->SetRendered(so.at("enabled"));
      }

      std::shared_ptr<ModelMaterial> modelMat = std::dynamic_pointer_cast<ModelMaterial>(
          loadedMaterials.at(materialID));

      modelMat->RegisterModel(model);

    }
    
    // TODO: render something to debug lighting positions
    if (soType == "light") {
      std::cout << "Loading light: " << so.at("name") << std::endl;

      glm::vec3 lpos;
      if (so.find("position") != so.end()) {
        auto pos = so.at("position");
        std::cout << "Light data: " << pos << std::endl;
        lpos = glm::vec3(pos[0], pos[1], pos[2]);
      }

      auto light = Make<UniSceneObject>(lpos, (std::string)so.at("name"));

      if (so.find("rotation") != so.end()) {
        auto rot = so.at("rotation");
        light->GetTransform()->SetRotation({rot[0], rot[1], rot[2]});
      }

      auto radius = so.at("radius");
      auto colarray = so.at("color");
      auto color =
          glm::vec4(colarray[0], colarray[1], colarray[2], colarray[3]);
      auto enabled = so.at("on");
      light->AddComponent<LightComponent>(radius, color, enabled);

      if (so.find("components") != so.end()) {
        auto components = so.at("components");
        if (components.find("movement") != components.end()) {
          auto movement = components.at("movement");
          auto vel = glm::vec3(0);
          auto rot = glm::vec3(0);

          if (movement.find("velocity") != movement.end()) {
            auto velocity = movement.at("velocity");
            vel = glm::vec3(velocity.at(0), velocity.at(1), velocity.at(2));
          }

          if (movement.find("rotation") != movement.end()) {
            auto rotation = movement.at("rotation");
            rot = glm::vec3(rotation.at(0), rotation.at(1), rotation.at(2));
          }

          light->AddComponent<MovementComponent>(vel, rot);
        }
      }
    }
    if (soType == "planet") {
      std::cout << "Loading planet: " << so.at("name") << std::endl;

      glm::vec3 ppos;
      if (so.find("position") != so.end()) {
        auto pos = so.at("position");
        ppos = glm::vec3(pos[0], pos[1], pos[2]);
      }

      auto radius = so.at("radius");
      auto maxHeight = so.at("maxHeight");
      auto maxDepth = so.at("maxDepth");
      auto gridSize = 100;
      if (so.find("gridSize") != so.end()) {
        gridSize = so.at("gridSize");
      }
      auto hasOcean = false;
      if (so.find("hasOcean") != so.end()) {
        hasOcean = so.at("hasOcean");
      }

      std::string name = so.at("name");

      auto planet = Make<UniSceneObject>(ppos, name);
      planet->AddComponent<UniPlanet>(radius, maxHeight, maxDepth, gridSize,
                                      hasOcean);

      auto density = so.at("density");

      planet->AddComponent<PhysicsComponent>(radius, density, true);
      planet->m_Entity->get<PhysicsComponent>()->SetSceneObject(planet);

      if (so.find("rotation") != so.end()) {
        auto rot = so.at("rotation");
        planet->GetTransform()->SetRotation({rot[0], rot[1], rot[2]});
      }
      planet->GetComponent<UniPlanet>()->AddNoiseLayer(UniPlanet::SIMPLEX, 1);
      planet->GetComponent<UniPlanet>()->Initialize();
    }
  }

  auto playerStart = level.at("playerStart");
  auto playerPos = playerStart.at("position");
  auto playerRot = playerStart.at("rotation");

  auto camObj = GetCameraObject();
  camObj->GetTransform()->SetPosition(
      glm::vec3(playerPos.at(0), playerPos.at(1), playerPos.at(2)));
  camObj->GetTransform()->SetRotation(
      {playerRot[0], playerRot[1], playerRot[2]});
  GetCameraComponent()->CalculateView(camObj->GetTransform());

  std::cout << "Scene fully loaded." << std::endl;
}

void UniScene::Unload() {
  // Meshes
  auto models = GetModels();
  for_each(models.begin(), models.end(),
           [](std::shared_ptr<UniModel> model) { model->Destroy(); });
}

void UniScene::Tick(float deltaTime) {
  m_World->tick(deltaTime);
  // m_BodyTest->Update();
}

std::vector<std::shared_ptr<UniModel>> UniScene::GetModels() {
  if (!m_ModelCache.empty()) {
    return m_ModelCache;
  }

  std::vector<std::shared_ptr<UniModel>> models;
  for_each(m_SceneObjects.begin(), m_SceneObjects.end(),
           [&models](std::shared_ptr<UniSceneObject> so) {
             auto model = std::dynamic_pointer_cast<UniModel>(so);
             if (model && model->IsRendered()) {
               models.push_back(model);
             }
           });

  return models;
}

std::vector<std::shared_ptr<UniSceneObject>> UniScene::GetRenderedObjects() {
  if (!m_RenderedObjectCache.empty()) {
    return m_RenderedObjectCache;
  }

  std::vector<std::shared_ptr<UniSceneObject>> objects;
  for_each(m_SceneObjects.begin(), m_SceneObjects.end(),
           [&objects](std::shared_ptr<UniSceneObject> so) {
             if (so->IsRendered()) {
               objects.push_back(so);
             }
           });

  return objects;
}

void UniScene::AddSceneObject(std::shared_ptr<UniSceneObject> so) {
  m_SceneObjects.push_back(so);
  m_ModelCache.clear();
  m_RenderedObjectCache.clear();
  m_ModelCache = GetModels();
}
