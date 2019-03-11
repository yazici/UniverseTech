#pragma once

#include <map>
#include <string>
#include <memory>
#include "Importer.h"
#include "Asset.h"


using namespace uni::import;

namespace uni::import
{
	
	class ImporterFactory {
	public:
	  void RegisterFactory(std::string assetType, std::shared_ptr<Importer> importer)
	  {
	    m_factories.insert(std::pair<std::string, std::shared_ptr<Importer>>(assetType, importer));
	  }
	
	  std::shared_ptr<uni::assets::Asset> Import(std::string assetType, std::shared_ptr<uni::assets::Asset> asset, bool force = false)
	  {
	    return m_factories.at(assetType)->Import(asset, force);
	  }
	
	  std::shared_ptr<uni::assets::Asset> LoadAsset(std::string assetType, json data)
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
	  std::unordered_map<std::string, std::shared_ptr<Importer>> m_factories;
	};
}

namespace uni::assets
{
	
	class AssetManager
	{
	public:
	  AssetManager(std::string basePath = "", std::string registryFile = "/assets.json");
	  ~AssetManager() = default;
	
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
	  std::map<std::string, std::shared_ptr<Asset>> m_assets;
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
	  ReturnType RegisterAsset(std::string path, std::shared_ptr<Asset> asset, bool replace);
	  // Registers an asset with a path name.
	
	  void CheckImported(std::vector<std::string> assets);
	
	
	  static std::shared_ptr<ImporterFactory> GetRegistry() {
	    const static std::shared_ptr<ImporterFactory> importerRegistry = std::make_shared<ImporterFactory>();
	    return importerRegistry;
	  }
	
	  static void RegisterImporter(std::string assetType, std::shared_ptr<Importer> importer) {
	    GetRegistry()->RegisterFactory(assetType, importer);
	  }
	
	  std::shared_ptr<Asset> Import(std::shared_ptr<Asset> asset) {
	    return GetRegistry()->Import(asset->m_type, asset);
	  }
	
	  std::shared_ptr<Asset> Import(std::string assetPath) {
	    auto asset = GetAsset(assetPath);
	    return GetRegistry()->Import(asset->m_type, asset);
	  }
	
	
	  std::shared_ptr<Asset> Import(std::string assetType, std::shared_ptr<Asset> asset) {
	    return GetRegistry()->Import(assetType, asset);
	  }
	
	  std::shared_ptr<Asset> LoadAsset(std::string assetType, json data) {
	    return GetRegistry()->LoadAsset(assetType, data);
	  }
	
	  std::string GetPath() { return m_basePath; }
	
	  bool ImportAll();
	
	
	  // Deletes an asset with a path name.
	  ReturnType DeleteAsset(std::string path);
	  // Returns an asset at a given path
	  std::shared_ptr<Asset> GetAsset(std::string path);
	
	  template<typename T>
	  std::shared_ptr<T> GetAsset(std::string path);
	};
}


template<typename T>
std::shared_ptr<T> uni::assets::AssetManager::GetAsset(std::string path) {
  if (path.empty())
    return nullptr;
  //std::cout << "Retrieving asset " + path << std::endl;
  std::shared_ptr<Asset> asset = GetAsset(path);
  //std::cout << "Got UniAsset at " << asset->m_path << std::endl;
  std::shared_ptr<T> ret = std::dynamic_pointer_cast<T>(asset);
  return ret;
}

