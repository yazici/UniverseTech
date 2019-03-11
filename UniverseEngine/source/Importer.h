#pragma once

#include <string>
#include <memory>
#include "components/LightComponent.h"
#include <nlohmann/json.hpp>
#include "vks/VulkanTools.h"

using json = nlohmann::json;

namespace uni {
  namespace assets {
    class Asset;
  }
}


namespace uni::import {

  class Importer
  {
  public:
    Importer() = default;
    virtual ~Importer() = default;
    virtual std::shared_ptr<uni::assets::Asset> Import(std::shared_ptr<uni::assets::Asset> asset, bool force = false) = 0;
    virtual std::shared_ptr<uni::assets::Asset> LoadAsset(json data) = 0;

    template<typename T>
    std::shared_ptr<T> CreateAsset(json data);


  protected:
    std::string m_name;
  };


  class Texture2DImporter : public Importer {
  public:
    Texture2DImporter() = default;
    std::shared_ptr<uni::assets::Asset> Import(std::shared_ptr<uni::assets::Asset> asset, bool force = false) override;
    std::shared_ptr<uni::assets::Asset> LoadAsset(json data) override;
  };


  class ModelImporter : public Importer {
  public:
    ModelImporter() = default;
    std::shared_ptr<uni::assets::Asset> Import(std::shared_ptr<uni::assets::Asset> asset, bool force = false) override;
    std::shared_ptr<uni::assets::Asset> LoadAsset(json data) override;
  };

  class MaterialImporter : public Importer {
  public:
    MaterialImporter() = default;
    std::shared_ptr<uni::assets::Asset> Import(std::shared_ptr<uni::assets::Asset> asset, bool force = false) override;
    std::shared_ptr<uni::assets::Asset> LoadAsset(json data) override;
  };

  class AudioImporter : public Importer {
  public:
    AudioImporter() = default;
    std::shared_ptr<uni::assets::Asset> Import(std::shared_ptr<uni::assets::Asset> asset, bool force = false) override;
    std::shared_ptr<uni::assets::Asset> LoadAsset(json data) override;
  };

  template<typename T>
  std::shared_ptr<T> Importer::CreateAsset(json data) {

    auto engine = UniEngine::GetInstance();

    std::string aType = data.at("type");
    std::string aPath = data.at("path");
    long aUpdated = data.at("last_update");
    std::string aSource = "";
    if (data.find("filename") != data.end())
      aSource =  engine->getAssetPath() + static_cast<std::string>(data.at("filename"));

    auto asset = std::make_shared<T>(aType, aPath);

    asset->m_lastUpdate = aUpdated;
    asset->m_sourceFile = aSource;

    asset->m_settings = data;

    return asset;
  }

}