#pragma once

#include "../UniMaterial.h"

class ModelComponent;

class ModelMaterial : public UniMaterial {
 public:
  virtual ~ModelMaterial() = default;

  std::vector<std::shared_ptr<ModelComponent>> GetModels() { return m_models; }

 private:
  std::vector<std::shared_ptr<ModelComponent>> m_models;


 public:
  ModelMaterial() = default;
  ModelMaterial(std::string name);
};
