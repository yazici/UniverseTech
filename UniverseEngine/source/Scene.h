#pragma once
#include <vector>
#include <memory>
#include "ECS.h"
#include "SceneObject.h"
#include "components/Components.h"


class UniEngine;
using namespace uni::components;

namespace uni
{
	namespace scene
	{
		
		class Scene {
		public:
			Scene();
		  Scene(std::string name);
			~Scene();
		
			ECS::ComponentHandle<CameraComponent> GetCameraComponent() { return m_CurrentCamera->m_Entity->get<CameraComponent>(); }
		
			void Initialize();
		
			void Load(std::string filename);
		
			void Unload();
		
			template <class T> std::shared_ptr<T> Make(std::shared_ptr<T> so);
			template<class _Ty0, class... _Types> std::shared_ptr<_Ty0> Make(glm::vec3 pos, _Types&&... _Args);
		
			std::shared_ptr<SceneObject> GetCameraObject() { return m_CurrentCamera; }
		
			ECS::World* m_World;
		
			void Tick(float deltaTime);
		
			std::vector<std::shared_ptr<SceneObject>> m_SceneObjects;
			std::vector<std::shared_ptr<SceneObject>> m_RenderedObjectCache;
		
			std::vector<std::shared_ptr<SceneObject>> GetRenderedObjects();
			void AddSceneObject(std::shared_ptr<SceneObject> so);
		
			template <class T> std::vector<std::shared_ptr<T>> GetObjectsOfClass();
		
		
			std::string GetName() { return m_Name; }
		private:
			std::shared_ptr<SceneObject> m_CurrentCamera;
			std::string m_Name;
		};
		
		
		template <class T> 
		std::shared_ptr<T> Scene::Make(std::shared_ptr<T> so) {
			so->SetScene(this);
			so->SetEntity(m_World->create());
			AddSceneObject(so);
			return so;
		}
		
		template<class T, class... _Types> 
		std::shared_ptr<T> Scene::Make(glm::vec3 pos, _Types&&... _Args) {
			std::shared_ptr<T> so = std::make_shared<T>(_Args...);
			so->SetScene(this);
			so->SetEntity(m_World->create(), pos);
			AddSceneObject(so);
			return so;
		}
		
		template <class T> std::vector<std::shared_ptr<T>> Scene::GetObjectsOfClass() {
		
			std::vector<std::shared_ptr<T>> objects;
		
			for_each(m_SceneObjects.begin(), m_SceneObjects.end(), [&objects](std::shared_ptr<SceneObject> so) {
				auto object = std::dynamic_pointer_cast<T>(so);
				if(object) {
					objects.push_back(object);
				}
			});
		
			return objects;
		}
	}
}