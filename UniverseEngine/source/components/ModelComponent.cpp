#include "ModelComponent.h"
#include "../UniEngine.h"
#include "../UniAssetManager.h"
#include "../UniAsset.h"

ModelComponent::ModelComponent(std::string path) {
  auto asset = UniEngine::GetInstance()->AssetManager()->GetAsset<UniAssetModel>(path);
  m_Model = asset->m_model;
  m_Name = path;
}

ModelComponent::~ModelComponent() {}
