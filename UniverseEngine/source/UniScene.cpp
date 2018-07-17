#include "UniScene.h"
#include <memory>
#include "systems/Systems.h"
#include "UniEngine.h"
#include "components/UniPlanet.h"
#include "components/PlayerMovement.h"
#include "components/LightComponent.h"
#include <nlohmann/json.hpp>
#include <iosfwd>
#include "systems/PlanetRenderSystem.h"
#include "systems/PlayerControlSystem.h"


using json = nlohmann::json;

UniScene::UniScene() {}

UniScene::~UniScene() {
	m_World->destroyWorld();
}

void UniScene::Initialize(UniEngine* engine) {
	m_World = ECS::World::createWorld();
	m_World->registerSystem(new MovementSystem());
	m_World->registerSystem(new CameraSystem());
	m_World->registerSystem(new PlanetRenderSystem());
	m_World->registerSystem(new PlayerControlSystem());
	

	m_CurrentCamera = Make<UniSceneObject>(glm::vec3(0));
	m_CurrentCamera->AddComponent<CameraComponent>(m_CurrentCamera->GetTransform(), (float)engine->width / (float)engine->height, 60.f, 0.1f, 1000.0f);

	m_CurrentCamera->AddComponent<MovementComponent>();
	m_CurrentCamera->AddComponent<PlayerControlComponent>();
	//m_CurrentCamera->m_Entity->get<PlayerControlComponent>()->SetTarget(glm::vec3(0, 0, 0));
}

void UniScene::Load(std::string filename) {

	UniEngine& engine = UniEngine::GetInstance();

	std::ifstream t(filename);
	std::stringstream buffer;
	buffer << t.rdbuf();

	json data = json::parse(buffer.str());
	auto level = data["level"];
	m_Name = level["name"];

	for(const auto& so : level.at("sceneObjects")) {
		std::string soType = so.at("type");
		if(soType == "model") {
			std::cout << "Loading model: " << so.at("name") << std::endl;
			// TODO: Load UniModel from JSON data.
			// 	m_Name = jsonData["name"];
			auto modelPath = so.at("mesh");
			auto texturePath = so.at("texture");
			auto normalMapPath = so.at("normal");

			glm::vec3 mpos;
			if(so.find("position") != so.end()) {
				auto pos = so.at("position");
				mpos = glm::vec3(pos[0], pos[1], pos[2]);
			}

			std::cout << "Creating model path: " << modelPath << ", texture: " << texturePath << ", normals: " << normalMapPath << std::endl;
			auto model = Make<UniModel>(mpos, modelPath, texturePath, normalMapPath);

			
			if(so.find("rotation") != so.end()) {
				auto rot = so.at("rotation");
				model->GetTransform()->SetPitch(rot[0]);
				model->GetTransform()->SetYaw(rot[1]);
				model->GetTransform()->SetRoll(rot[2]);
			}

			glm::vec3 createScale = { so.at("createScale")[0], so.at("createScale")[1], so.at("createScale")[2] };
			glm::vec2 createUVScale = { so.at("createUVScale")[0], so.at("createUVScale")[1] };
			glm::vec3 createOffset = glm::vec3(0);

			if(so.find("createOffset") != so.end()) {
				createOffset = { so.at("createOffset")[0], so.at("createOffset")[1], so.at("createOffset")[2] };
			}

			if(so.find("components") != so.end()) {
				auto components = so.at("components");
				if(components.find("movement") != components.end()) {
					auto movement = components.at("movement");
					auto vel = glm::vec3(0);
					auto rot = glm::vec3(0);

					if(movement.find("velocity") != movement.end()) {
						auto velocity = movement.at("velocity");
						vel = glm::vec3(velocity.at(0), velocity.at(1), velocity.at(2));
					}

					if(movement.find("rotation") != movement.end()) {
						auto rotation = movement.at("rotation");
						rot = glm::vec3(rotation.at(0), rotation.at(1), rotation.at(2));
					}

				model->AddComponent<MovementComponent>(vel, rot);
					
				}
			}
			
			model->SetCreateInfo(createOffset, createScale, createUVScale);
			model->Load(engine.vertexLayout, engine.vulkanDevice, engine.GetQueue(), true);
		}

		if(soType == "light") {
			std::cout << "Loading light: " << so.at("name") << std::endl;

			glm::vec3 lpos;
			if(so.find("position") != so.end()) {
				auto pos = so.at("position");
				lpos = glm::vec3(pos[0], pos[1], pos[2]);
			}

			auto light = Make<UniSceneObject>(lpos);

			
			if(so.find("rotation") != so.end()) {
				auto rot = so.at("rotation");
				light->GetTransform()->SetPitch(rot[0]);
				light->GetTransform()->SetYaw(rot[1]);
				light->GetTransform()->SetRoll(rot[2]);
			}

			auto radius = so.at("radius");
			auto colarray = so.at("color");
			auto color = glm::vec4(colarray[0], colarray[1], colarray[2], colarray[3]);
			auto enabled = so.at("on");
			light->AddComponent<LightComponent>(radius, color, enabled);


			if(so.find("components") != so.end()) {
				auto components = so.at("components");
				if(components.find("movement") != components.end()) {
					auto movement = components.at("movement");
					auto vel = glm::vec3(0);
					auto rot = glm::vec3(0);

					if(movement.find("velocity") != movement.end()) {
						auto velocity = movement.at("velocity");
						vel = glm::vec3(velocity.at(0), velocity.at(1), velocity.at(2));
					}

					if(movement.find("rotation") != movement.end()) {
						auto rotation = movement.at("rotation");
						rot = glm::vec3(rotation.at(0), rotation.at(1), rotation.at(2));
					}

					light->AddComponent<MovementComponent>(vel, rot);

				}
			}

		}
	}

	auto playerStart = level.at("playerStart");
	auto playerPos = playerStart.at("position");
	auto playerRot = playerStart.at("rotation");

	auto camObj = GetCameraObject();
	camObj->GetTransform()->SetPosition(glm::vec3(playerPos.at(0), playerPos.at(1), playerPos.at(2)));
	GetCameraComponent()->CalculateView(camObj->GetTransform());


	auto planetTest = Make<UniSceneObject>(glm::vec3(0, 0, -5));
	planetTest->AddComponent<UniPlanet>(10.0);


	std::cout << "Scene fully loaded." << std::endl;
}

void UniScene::Tick(float deltaTime) {
	m_World->tick(deltaTime);
	//m_BodyTest->Update();
}

std::vector<std::shared_ptr<UniModel>> UniScene::GetModels() {

	if(!m_ModelCache.empty()) {
		return m_ModelCache;
	}

	std::vector<std::shared_ptr<UniModel>> models;
	for_each(m_SceneObjects.begin(), m_SceneObjects.end(), [&models](std::shared_ptr<UniSceneObject> so) {
		auto model = std::dynamic_pointer_cast<UniModel>(so);
		if(model) {
			models.push_back(model);
		}
	});

	return models;
}

std::vector<std::shared_ptr<UniSceneObject>> UniScene::GetRenderedObjects() {

	if(!m_RenderedObjectCache.empty()) {
		return m_RenderedObjectCache;
	}

	std::vector<std::shared_ptr<UniSceneObject>> objects;
	for_each(m_SceneObjects.begin(), m_SceneObjects.end(), [&objects](std::shared_ptr<UniSceneObject> so) {
		if(so->IsRendered()) {
			objects.push_back(so);
		}
	});

	return objects;
}



void UniScene::AddSceneObject(std::shared_ptr<UniSceneObject> so)
{
	m_SceneObjects.push_back(so);
	m_ModelCache.clear();
	m_RenderedObjectCache.clear();
	m_ModelCache = GetModels();
}

