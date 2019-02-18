#include "ModelMaterial.h"

#include <array>
#include "../UniEngine.h"
#include "../UniSceneManager.h"
#include "../UniSceneRenderer.h"

ModelMaterial::ModelMaterial(std::string name) {
  auto& engine = UniEngine::GetInstance();
  auto aPath = engine.getAssetPath();
  m_Name = name;
  SetShader("vert", aPath + "shaders/omnishader.vert.spv");
  SetShader("frag", aPath + "shaders/omnishader.frag.spv");
}


void ModelMaterial::Destroy() {
  std::cout << "Destroying model material..." << std::endl;

  for (auto& tex : m_Textures) {
    tex->destroy();
  }
}

void ModelMaterial::LoadTexture(std::string name, std::string texturePath) {

  //if (texturePath.empty()) {
  //  return;
  //}

  if (name == "texture") {
    m_hasTextureMap = true;
  }
  if (name == "normal") {
    m_hasNormalMap = true;
  }
  if (name == "metallic") {
    m_hasMetallicMap = true;
  }
  if (name == "roughness") {
    m_hasRoughnessMap = true;
  }
  if (name == "specular") {
    m_hasSpecularMap = true;
  }
  if (name == "emissive") {
    m_hasEmissiveMap = true;
  }
  if (name == "ao") {
    m_hasAOMap = true;
  }

  UniMaterial::LoadTexture(name, texturePath);
  
}
