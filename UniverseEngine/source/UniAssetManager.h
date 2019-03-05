#pragma once

#include <map>
#include <string>
#include <memory>
#include "UniImporter.h"

class UniAsset;



class UniImporterFactory {
public:
  void RegisterFactory(std::string assetType, std::shared_ptr<UniImporter> importer)
  {
    m_factories.insert(std::pair<std::string, std::shared_ptr<UniImporter>>(assetType, importer));
  }

  std::shared_ptr<UniAsset> Import(std::string assetType, json data, json settings = nullptr)
  {
    return m_factories.at(assetType)->Import(data, settings);
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
  UniAssetManager(std::string basePath = "", std::string registryFile = "assets.json");
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

  UniImporterFactory m_ImporterRegistry;


public:
  void Shutdown();
  // Loads the registry from a file.
  ReturnType LoadRegistry();
  // Saves the registry to a file.
  ReturnType SaveRegistry();
  // Registers an asset with a path name.
  ReturnType RegisterAsset(std::string path, std::shared_ptr<UniAsset> asset, bool replace);
  // Registers an asset with a path name.
  
  
  void RegisterImporter(std::string assetType, std::shared_ptr<UniImporter> importer) {
    m_ImporterRegistry.RegisterFactory(assetType, importer);
  }

  std::shared_ptr<UniAsset> Import(std::string assetType, json data, json settings) {
    return m_ImporterRegistry.Import(assetType, data, settings);
  }
  
  // Deletes an asset with a path name.
  ReturnType DeleteAsset(std::string path);
  // Returns an asset at a given path
  std::shared_ptr<UniAsset> GetAsset(std::string path);
  // Loads an asset's data into memory so it can be spawned and used in game.
  std::shared_ptr<UniAsset> LoadAsset(std::string path);
};

