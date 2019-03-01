#pragma once

#include <vector>
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "ECS.h"
#include "components/Components.h"
#include "glm/glm.hpp"

class UniScene;

class UniSceneObject {
 public:
  UniSceneObject(std::string name = "unnamed object") { m_Name = name; }
  virtual ~UniSceneObject() = default;

  std::shared_ptr<UniSceneObject> m_Parent;

  void SetScene(UniScene* scene);
  void SetEntity(ECS::Entity* ent, glm::vec3 pos);
  void SetParent(std::shared_ptr<UniSceneObject> parent);

  ECS::Entity* m_Entity = nullptr;

  ECS::ComponentHandle<TransformComponent> GetTransform();

  template <class T, class... _Types>
  ECS::ComponentHandle<T> AddComponent(_Types&&... _Args);

  template <class T>
  ECS::ComponentHandle<T> GetComponent();

  std::string GetName() { return m_Name; }
  void SetName(std::string name) { m_Name = name; }

  bool IsRendered() { return m_IsRendered; }
  void SetRendered(bool render) { m_IsRendered = render; }
  void SetRenderIndex(uint32_t index) { m_renderIndex = index; }
  uint32_t GetRenderIndex() { return m_renderIndex; }

  void Destroy();

 protected:
  UniScene* m_Scene;
  std::string m_Name = "object";
  bool m_IsRendered = false;
  uint32_t m_renderIndex = 0;
};

template <class T, class... _Types>
ECS::ComponentHandle<T> UniSceneObject::AddComponent(_Types&&... _Args) {
  return m_Entity->assign<T>(_Args...);
}

template <class T>
ECS::ComponentHandle<T> UniSceneObject::GetComponent() {
  return m_Entity->get<T>();
}
