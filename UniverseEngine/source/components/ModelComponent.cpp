#include "ModelComponent.h"
#include "../UniEngine.h"
#include "../AssetManager.h"
#include "../Asset.h"

uni::components::ModelComponent::ModelComponent(std::string path) {
  auto asset = UniEngine::GetInstance()->GetAssetManager()->GetAsset<uni::assets::UniAssetModel>(path);
  m_Model = asset->m_model;
  m_Name = path;
}

uni::components::ModelComponent::~ModelComponent() {}
