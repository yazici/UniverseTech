#pragma once

#include <chrono>
#include <string>
#include <nlohmann/json.hpp>
#include "vks/VulkanTexture.hpp"
#include "UniModelMesh.h"
#include "Materials.h"

using json = nlohmann::json;

namespace uni::assets {

  class UniAsset {
  public:
    UniAsset() = default;
    UniAsset(std::string t, std::string p) : m_type(t), m_path(p) {
    };
    virtual void SetFile(std::string f) { m_sourceFile = f; }
    virtual ~UniAsset() = default;
    virtual void Destroy() {}
    virtual bool Load() { return true; }

    std::string m_type;
    std::string m_path;
    std::string m_sourceFile = "";
    uint64_t m_lastUpdate = 0;
    uint64_t m_created = 0;

    json m_settings = nullptr;

    bool m_isLoaded = false;

  };

  class UniAssetTexture2D : public UniAsset {
  public:
    UniAssetTexture2D(std::string t, std::string p) : UniAsset(t, p) {}
    std::shared_ptr<vks::Texture2D> m_texture;

    void Destroy() override {
      m_texture->destroy();
    }
  };


  class UniAssetModel : public UniAsset {
  public:
    UniAssetModel(std::string t, std::string p) : UniAsset(t, p) {}
    std::shared_ptr<uni::Model> m_model;
    std::vector<std::string> m_materials;

    void Destroy() override {
      m_model->destroy();
    }
  };


  class UniAssetMaterial : public UniAsset {
  public:
    UniAssetMaterial(std::string t, std::string p) : UniAsset(t, p) {}
    std::shared_ptr<ModelMaterial> m_material;

    void Destroy() override {
      std::cout << "Destroying material for asset " << m_path << std::endl;
      m_material->Destroy();
    }
  };



  class UniAssetAudio : public UniAsset {
  public:
    UniAssetAudio(std::string t, std::string p) : UniAsset(t, p) {}
    void Destroy() override;
    bool m_is3d = true;
    bool m_isLooping = false;
    float m_Volume = 50.f;
    bool m_isStreaming = false;
  };

}