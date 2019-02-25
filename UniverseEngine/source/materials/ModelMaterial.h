#pragma once

#include "../UniMaterial.h"

class ModelComponent;

class ModelMaterial : public UniMaterial {
 public:
  virtual ~ModelMaterial() { Destroy(); }

  std::vector<std::shared_ptr<ModelComponent>> GetModels() { return m_models; }

 private:
  std::string m_Name;
  std::vector<std::shared_ptr<ModelComponent>> m_models;


 public:
  ModelMaterial() = default;
  ModelMaterial(std::string name);
};
