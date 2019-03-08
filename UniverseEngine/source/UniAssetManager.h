#pragma once

#include <map>
#include <string>
#include <memory>
#include "UniImporter.h"
#include "UniAsset.h"


using namespace uni::import;

class UniImporterFactory {
public:
  void RegisterFactory(std::string assetType, std::shared_ptr<UniImporter> importer)
  {
    m_factories.insert(std::pair<std::string, std::shared_ptr<UniImporter>>(assetType, importer));
  }

  std::shared_ptr<UniAsset> Import(std::string assetType, std::shared_ptr<UniAsset> asset, bool force = false)
  {
    return m_factories.at(assetType)->Import(asset, force);
  }

  std::shared_ptr<UniAsset> LoadAsset(std::string assetType, json data)
  {
    return m_factories.at(assetType)->LoadAsset(data);
  }

  std::list<std::string> GetTypes()
  {
    std::list<std::string> result;
    for (auto& item : m_factories)
    {
      result.push_back(item.first);
    }
    return result;
  }

private:
  std::unordered_map<std::string, std::shared_ptr<UniImporter>> m_factories;
};


class UniAssetManager
{
public:
  UniAssetManager(std::string basePath = "", std::string registryFile = "/assets.json");
  ~UniAssetManager() = default;

  enum ReturnType {
    CREATED_OK,
    ALREADY_CREATED,
    CREATE_FAILED,
    LOAD_OK,
    LOAD_FAILED,
    DELETED_OK,
    FILE_NOT_FOUND,
    PATH_EXISTS
  };

  const std::string GetAssetPath(std::string path) {
    return m_basePath + path;
  }

private:
  std::map<std::string, std::shared_ptr<UniAsset>> m_assets;
  std::string m_basePath;
  std::string m_registryFile;
  ReturnType CheckPath(std::string path);




public:
  void Shutdown();
  // Loads the registry from a file.
  ReturnType LoadRegistry();
  // Saves the registry to a file.
  ReturnType SaveRegistry();
  // Registers an asset with a path name.
  ReturnType RegisterAsset(std::string path, std::shared_ptr<UniAsset> asset, bool replace);
  // Registers an asset with a path name.

  void CheckImported(std::vector<std::string> assets);


  static std::shared_ptr<UniImporterFactory> GetRegistry() {
    const static std::shared_ptr<UniImporterFactory> importerRegistry = std::make_shared<UniImporterFactory>();
    return importerRegistry;
  }

  static void RegisterImporter(std::string assetType, std::shared_ptr<UniImporter> importer) {
    GetRegistry()->RegisterFactory(assetType, importer);
  }

  std::shared_ptr<UniAsset> Import(std::shared_ptr<UniAsset> asset) {
    return GetRegistry()->Import(asset->m_type, asset);
  }

  std::shared_ptr<UniAsset> Import(std::string assetPath) {
    auto asset = GetAsset(assetPath);
    return GetRegistry()->Import(asset->m_type, asset);
  }


  std::shared_ptr<UniAsset> Import(std::string assetType, std::shared_ptr<UniAsset> asset) {
    return GetRegistry()->Import(assetType, asset);
  }

  std::shared_ptr<UniAsset> LoadAsset(std::string assetType, json data) {
    return GetRegistry()->LoadAsset(assetType, data);
  }

  std::string GetPath() { return m_basePath; }

  bool ImportAll();


  // Deletes an asset with a path name.
  ReturnType DeleteAsset(std::string path);
  // Returns an asset at a given path
  std::shared_ptr<UniAsset> GetAsset(std::string path);

  template<typename T>
  std::shared_ptr<T> GetAsset(std::string path);
};


template<typename T>
std::shared_ptr<T> UniAssetManager::GetAsset(std::string path) {
  if (path.empty())
    return nullptr;
  //std::cout << "Retrieving asset " + path << std::endl;
  std::shared_ptr<UniAsset> asset = GetAsset(path);
  //std::cout << "Got UniAsset at " << asset->m_path << std::endl;
  std::shared_ptr<T> ret = std::dynamic_pointer_cast<T>(asset);
  return ret;
}

