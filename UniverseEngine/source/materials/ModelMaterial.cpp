#include "ModelMaterial.h"

#include <array>
#include "../UniEngine.h"
#include "../UniSceneManager.h"
#include "../UniSceneRenderer.h"

ModelMaterial::ModelMaterial(std::string name) {
  auto engine = UniEngine::GetInstance();
  auto aPath = engine->getAssetPath();
  m_Name = name;
  SetShader("vert", aPath + "shaders/omnishader.vert.spv");
  SetShader("frag", aPath + "shaders/omnishader.frag.spv");
}