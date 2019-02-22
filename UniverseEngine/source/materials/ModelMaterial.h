#pragma once

#include "../UniMaterial.h"

class UniModel;

class ModelMaterial : public UniMaterial {
 public:
  virtual ~ModelMaterial() { Destroy(); }

  std::vector<std::shared_ptr<UniModel>> GetModels() { return m_models; }

 private:
  std::string m_Name;
  std::vector<std::shared_ptr<UniModel>> m_models;


 public:
  ModelMaterial() = default;
  ModelMaterial(std::string name);
};

using ModelMaterialFactory =
    Factory<std::string,
            std::shared_ptr<ModelMaterial>>::Initializer<std::string>;
