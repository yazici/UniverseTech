#pragma once
#include <vector>
#include <memory>
#include "ECS.h"
#include "UniSceneObject.h"
#include "UniModel.h"


class UniBody;
class UniEngine;

class UniScene {
public:
	UniScene();
	~UniScene();

	ECS::ComponentHandle<CameraComponent> GetCameraComponent() { return m_CurrentCamera->m_Entity->get<CameraComponent>(); }

	void Initialize(UniEngine* engine);
	void Load();

	void Load(std::string filename);

	template <class T> std::shared_ptr<T> Make(std::shared_ptr<T> so);
	template<class _Ty0, class... _Types> std::shared_ptr<_Ty0> Make(_Types&&... _Args);

	std::shared_ptr<UniSceneObject> GetCameraObject() { return m_CurrentCamera; }

	ECS::World* m_World;

	void Tick(float deltaTime);

	std::vector<std::shared_ptr<UniSceneObject>> m_SceneObjects;
	std::vector<std::shared_ptr<UniModel>> m_ModelCache;

	std::vector<std::shared_ptr<UniModel>> GetModels();
	void AddSceneObject(std::shared_ptr<UniSceneObject> so);

	std::shared_ptr<UniBody> m_BodyTest;

	std::string GetName() { return m_Name; }
private:
	std::shared_ptr<UniSceneObject> m_CurrentCamera;
	std::string m_Name;
};


template <class T> 
std::shared_ptr<T> UniScene::Make(std::shared_ptr<T> so) {
	so->SetScene(this);
	so->SetEntity(m_World->create());
	AddSceneObject(so);
	return so;
}

template<class T, class... _Types> 
std::shared_ptr<T> UniScene::Make(_Types&&... _Args) {
	std::shared_ptr<T> so = std::make_shared<T>(_Args...);
	so->SetScene(this);
	so->SetEntity(m_World->create());
	AddSceneObject(so);
	return so;
}
