#pragma once

#include "../UniMaterial.h"

class UniModel;

class ModelMaterial : public UniMaterial {
 public:
  virtual ~ModelMaterial() { Destroy(); }

  void LoadTexture(std::string name, short layer, std::string texturePath) override;

  std::vector<std::shared_ptr<UniModel>> GetModels() { return m_models; }

 private:
  std::string m_Name;
  std::vector<std::shared_ptr<UniModel>> m_models;

  bool m_hasTextureMap = false;
  bool m_hasNormalMap = false;
  bool m_hasRoughnessMap = false;
  bool m_hasSpecularMap = false;
  bool m_hasMetallicMap = false;
  bool m_hasEmissiveMap = false;
  bool m_hasAOMap = false;

 public:
  ModelMaterial() = default;
  ModelMaterial(std::string name, short layers = 0);
};

using ModelMaterialFactory =
    Factory<std::string,
            std::shared_ptr<ModelMaterial>>::Initializer<std::string>;
