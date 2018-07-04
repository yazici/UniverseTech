#include "UniScene.h"
#include <memory>
#include "Systems.h"
#include "UniBody.h"
#include "UniEngine.h"
#include "components/PlayerMovement.h"
#include <nlohmann/json.hpp>
#include <iosfwd>

using json = nlohmann::json;

UniScene::UniScene() {}

UniScene::~UniScene() {
	m_World->destroyWorld();
}

void UniScene::Initialize(UniEngine* engine) {
	m_World = ECS::World::createWorld();
	m_World->registerSystem(new MovementSystem());
	m_World->registerSystem(new CameraSystem());

	m_CurrentCamera = Make<UniSceneObject>();
	m_CurrentCamera->AddComponent<CameraComponent>(m_CurrentCamera->GetTransform(), (float)engine->width / (float)engine->height, 60.f, 0.1f, 1000.0f);

	m_CurrentCamera->AddComponent<MovementComponent>();
	m_CurrentCamera->AddComponent<PlayerControlComponent>();
	//m_CurrentCamera->m_Entity->get<PlayerControlComponent>()->SetTarget(glm::vec3(0, 0, 0));
}

void UniScene::Load() {

	

	UniEngine& engine = UniEngine::GetInstance();

	/*auto armor = Make<UniModel>("models/armor/armor.dae", "models/armor/color", "models/armor/normal");
	armor->SetName("armor");
	armor->GetTransform()->SetPosition(glm::vec3(0.0f, 0.0f, 10.0f));
	armor->GetTransform()->SetYaw(180.f);
	armor->AddComponent<MovementComponent>(glm::dvec3(0, 0, 5), glm::vec3(0, 90, 0));
	armor->SetCreateInfo(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f), glm::vec2(1.0f, 1.0f));
	armor->Load(engine.vertexLayout, engine.vulkanDevice, engine.GetQueue(), true);*/

	auto camObj = GetCameraObject();
	camObj->GetTransform()->SetPosition(1.f, -3.5f, -14.f);
	GetCameraComponent()->CalculateView(camObj->GetTransform());

	/*
	auto vgr = m_CurrentScene->Make<UniModel>("models/voyager/voyager.dae", "models/voyager/voyager", "");
	vgr->SetCreateInfo(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f), glm::vec2(1.0f, -1.0f));
	vgr->Load(vertexLayout, vulkanDevice, queue, true);
	*/

	/*auto floor = Make<UniModel>("models/openbox.dae", "textures/stonefloor02_color", "textures/stonefloor02_normal");
	floor->SetCreateInfo(glm::vec3(0.0f, -2.3f, 0.0f), glm::vec3(15.0f), glm::vec2(8.0f, 8.0f));
	floor->Load(engine.vertexLayout, engine.vulkanDevice, engine.GetQueue(), true);*/

	m_BodyTest = Make<UniBody>(7.0);
	m_BodyTest->Initialize();


}

void UniScene::Load(std::string filename) {

	UniEngine& engine = UniEngine::GetInstance();

	std::ifstream t(filename);
	std::stringstream buffer;
	buffer << t.rdbuf();

	json data = json::parse(buffer.str());
	auto level = data["level"];
	m_Name = level["name"];

	for(const auto& so : level["sceneObjects"]) {
		std::string soType = so["type"];
		if(soType == "model") {
			std::cout << "Loading model: " << so["name"] << std::endl;
			// TODO: Load UniModel from JSON data.
			// 	m_Name = jsonData["name"];
			auto modelPath = so.at("mesh");
			auto texturePath = so.at("texture");
			auto normalMapPath = so.at("normal");
			std::cout << "Creating model path: " << modelPath << ", texture: " << texturePath << ", normals: " << normalMapPath << std::endl;
			auto model = Make<UniModel>(modelPath, texturePath, normalMapPath);

			if(so.find("position") != so.end()) {
				auto pos = so.at("position");
				model->GetTransform()->SetPosition(glm::vec3(pos[0], pos[1], pos[2]));
			}
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


			//model->AddComponent<MovementComponent>(glm::dvec3(0, 0, 5), glm::vec3(0, 90, 0));
			
			model->SetCreateInfo(createOffset, createScale, createUVScale);
			model->Load(engine.vertexLayout, engine.vulkanDevice, engine.GetQueue(), true);
		}
	}

	//std::cout << data.dump() << std::endl;
}

void UniScene::Tick(float deltaTime) {
	m_World->tick(deltaTime);
	m_BodyTest->Update();
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

void UniScene::AddSceneObject(std::shared_ptr<UniSceneObject> so)
{
	m_SceneObjects.push_back(so);
	m_ModelCache.clear();
	m_ModelCache = GetModels();
}

