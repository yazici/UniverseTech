#include "UniScene.h"
#include <memory>
#include "Systems.h"
#include "UniBody.h"
#include "UniEngine.h"

UniScene::UniScene() {}

UniScene::~UniScene() {
	m_World->destroyWorld();
}

void UniScene::Initialize(UniEngine* engine) {
	m_World = ECS::World::createWorld();
	m_World->registerSystem(new MovementSystem());
	m_World->registerSystem(new CameraSystem());

	std::cout << "Unibody loading..." << std::endl;

	m_CurrentCamera = Make<UniSceneObject>();
	m_CurrentCamera->AddComponent<CameraComponent>(m_CurrentCamera->GetTransform(), (float)engine->width / (float)engine->height, 60.f, 0.1f, 1000.0f);

	m_BodyTest = Make<UniBody>(100.0);
	m_BodyTest->Initialize();

	std::cout << "Unibody loaded." << std::endl;
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

