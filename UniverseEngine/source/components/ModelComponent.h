#pragma once
#include <nlohmann/json.hpp>
#include <string>
#include "../3dmaths.h"
#include "../ModelMesh.h"
#include "../SceneObject.h"
//#include "vks/VulkanTexture.hpp"
//#include "vulkan/vulkan_core.h"
//#include "materials/ModelMaterial.h"

using json = nlohmann::json;

namespace uni
{
	namespace components
	{
		class ModelComponent {
		public:
		  ModelComponent() = default;
		  ModelComponent(std::string path);
		  void Destroy() {};
		  ~ModelComponent();
		
		  std::vector<std::string> m_Materials;
		  std::shared_ptr<uni::Model> m_Model;
		
		  void SetSceneObject(std::shared_ptr<uni::scene::SceneObject> so) {
		    m_SceneObject = so;
		  }
		  std::shared_ptr<uni::scene::SceneObject> GetSceneObject() { return m_SceneObject; }
		
		  std::vector<std::string> GetMaterialIDs() { return m_Materials; }
		
		  std::string GetName() { return m_Name; }
		
		protected:
		  std::string m_Name = "";
		  bool m_IsRendered = true;
		  std::shared_ptr<uni::scene::SceneObject> m_SceneObject;
		  
		};
	}
}
