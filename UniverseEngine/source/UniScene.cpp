#include "UniScene.h"
#include <memory>
#include "Systems.h"
#include "UniBody.h"
#include "UniEngine.h"
#include "components/PlayerMovement.h"

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
	m_CurrentCamera->m_Entity->get<PlayerControlComponent>()->SetTarget(glm::vec3(0, 0, 0));
}

void UniScene::Load() {
	UniEngine& engine = UniEngine::GetInstance();

	auto armor = Make<UniModel>("models/armor/armor.dae", "models/armor/color", "models/armor/normal");
	armor->SetName("armor");
	armor->GetTransform()->SetPosition(glm::vec3(0.0f, 0.0f, 10.0f));
	armor->GetTransform()->SetYaw(180.f);
	armor->AddComponent<MovementComponent>(glm::dvec3(0, 0, 5), glm::vec3(0, 90, 0));
	armor->SetCreateInfo(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f), glm::vec2(1.0f, 1.0f));
	armor->Load(engine.vertexLayout, engine.vulkanDevice, engine.GetQueue(), true);

	auto camObj = GetCameraObject();
	camObj->SetParent(armor);
	camObj->GetTransform()->SetPosition(1.f, 3.5f, -4.f);
	GetCameraComponent()->CalculateView(camObj->GetTransform());

	/*
	auto vgr = m_CurrentScene->Make<UniModel>("models/voyager/voyager.dae", "models/voyager/voyager", "");
	vgr->SetCreateInfo(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f), glm::vec2(1.0f, -1.0f));
	vgr->Load(vertexLayout, vulkanDevice, queue, true);
	*/

	auto floor = Make<UniModel>("models/openbox.dae", "textures/stonefloor02_color", "textures/stonefloor02_normal");
	floor->SetCreateInfo(glm::vec3(0.0f, -2.3f, 0.0f), glm::vec3(15.0f), glm::vec2(8.0f, 8.0f));
	floor->Load(engine.vertexLayout, engine.vulkanDevice, engine.GetQueue(), true);

	m_BodyTest = Make<UniBody>(7.0);
	m_BodyTest->Initialize();


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

