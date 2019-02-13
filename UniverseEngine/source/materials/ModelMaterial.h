#pragma once

#include "../UniMaterial.h"

class ModelMaterial : public UniMaterial {
 public:
  virtual ~ModelMaterial() { Destroy(); }

  uint32_t AddToCommandBuffer(VkCommandBuffer& cmdBuffer, uint32_t index, VkPipelineLayout layout) override;

  std::vector<std::shared_ptr<vks::Texture>> m_Textures;

  void Destroy() override;

 private:
  std::string m_Name;

 public:
  ModelMaterial() = default;
  ModelMaterial(std::string name);
};

using ModelMaterialFactory =
    Factory<std::string,
            std::shared_ptr<ModelMaterial>>::Initializer<std::string>;
