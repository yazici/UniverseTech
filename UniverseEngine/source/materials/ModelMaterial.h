#pragma once

#include "../UniMaterial.h"

class ModelComponent;

class ModelMaterial : public UniMaterial {
private:
  std::vector<std::shared_ptr<ModelComponent>> m_models;

public:
  std::vector<std::shared_ptr<ModelComponent>> GetModels() { return m_models; }

  ModelMaterial() = default;
  ModelMaterial(std::string name);
};
