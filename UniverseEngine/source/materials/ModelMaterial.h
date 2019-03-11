#pragma once

#include "../Material.h"


namespace uni {
  namespace components {
    class ModelComponent;
  }
  namespace materials {
    class ModelMaterial : public Material {
    private:
      std::vector<std::shared_ptr<uni::components::ModelComponent>> m_models;

    public:
      std::vector<std::shared_ptr<uni::components::ModelComponent>> GetModels() { return m_models; }

      ModelMaterial() = default;
      ModelMaterial(std::string name);
    };

  }
}



