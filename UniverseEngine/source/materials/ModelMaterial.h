#pragma once

#include "../Material.h"

class ModelComponent;

class ModelMaterial : public Material {
private:
  std::vector<std::shared_ptr<ModelComponent>> m_models;

public:
  std::vector<std::shared_ptr<ModelComponent>> GetModels() { return m_models; }

  ModelMaterial() = default;
  ModelMaterial(std::string name);
};
