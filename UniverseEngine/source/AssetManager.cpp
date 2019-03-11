#include "AssetManager.h"
#include "Asset.h"
#include "UniEngine.h"
#include <filesystem>
#include <iosfwd>
#include <nlohmann/json.hpp>
#include "vks/VulkanTools.h"
#include <ppl.h>

using json = nlohmann::json;
using namespace uni::assets;

AssetManager::AssetManager(std::string basePath, std::string registryFile)
{
  m_basePath = basePath;
  if (CheckPath(m_basePath) != PATH_EXISTS) {
    std::string err = "Asset path " + m_basePath + " does not exist.";
    vks::tools::exitFatal(err, -1);
  }
  if (CheckPath(m_basePath + registryFile) != PATH_EXISTS) {
    std::string err = "Registry file " + m_basePath + registryFile + " does not exist.";
    vks::tools::exitFatal(err, -1);
  }
  m_registryFile = registryFile;

}

AssetManager::ReturnType AssetManager::CheckPath(std::string path)
{
  if (std::filesystem::exists(path)) {
    return PATH_EXISTS;
  }
  return FILE_NOT_FOUND;
}

void AssetManager::Shutdown()
{
  for (auto& kv : m_assets) {
    kv.second->Destroy();
  }
}

AssetManager::ReturnType AssetManager::LoadRegistry()
{
  std::ifstream t(m_basePath + m_registryFile);
  std::stringstream buffer;
  buffer << t.rdbuf();

  json data = json::parse(buffer.str());

  auto registry = GetRegistry()->GetTypes();

  for (const auto& a : data.at("assets")) {
    if (std::find(registry.begin(), registry.end(), a.at("type")) == registry.end())
      continue;
    auto asset = GetRegistry()->LoadAsset(a.at("type"), a);
    m_assets.insert({ asset->m_path, asset });
  }

  return LOAD_OK;
}

// Registers an asset with a path name.
AssetManager::ReturnType AssetManager::RegisterAsset(std::string path, std::shared_ptr<Asset> asset, bool replace)
{
  if (m_assets.find(path) != m_assets.end() && !replace) {
    return ALREADY_CREATED;
  }

  m_assets.insert({ path, asset });
  return CREATED_OK;
}

void AssetManager::CheckImported(std::vector<std::string> assets)
{
  for (const auto& assetPath : assets) {
    auto _ = Import(assetPath);
  }
}

bool AssetManager::ImportAll()
{
  concurrency::parallel_for_each(m_assets.begin(), m_assets.end(), [&](auto & kv) {
  //for_each(m_assets.begin(), m_assets.end(), [&](auto & kv) {
    //std::cout << "Importing asset " << kv.first << std::endl;
    auto _ = GetRegistry()->Import(kv.second->m_type, kv.second);
    });

  return true;
}

AssetManager::ReturnType AssetManager::DeleteAsset(std::string path)
{
  m_assets.erase(path);
  return DELETED_OK;
}

std::shared_ptr<Asset> AssetManager::GetAsset(std::string path)
{
  if (m_assets.find(path) == m_assets.end()) {
    return nullptr;
  }
  return m_assets.at(path);
}
