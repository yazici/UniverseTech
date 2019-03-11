#include "ModelMaterial.h"

#include <array>
#include "../UniEngine.h"
#include "../SceneManager.h"
#include "../SceneRenderer.h"

ModelMaterial::ModelMaterial(std::string name) {

  std::cout << "### MODELMATERIAL " << name << " CREATED ###" << std::endl;
  
  auto engine = UniEngine::GetInstance();
  auto aPath = engine->getAssetPath();
  m_Name = name;
  SetShader("vert", aPath + "shaders/omnishader.vert.spv");
  SetShader("frag", aPath + "shaders/omnishader.frag.spv");
}