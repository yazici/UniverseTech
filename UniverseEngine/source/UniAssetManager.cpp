#include "UniAssetManager.h"
#include "UniAsset.h"
#include "UniEngine.h"
#include <filesystem>
#include <iosfwd>
#include <nlohmann/json.hpp>
#include "vks/VulkanTools.h"

using json = nlohmann::json;


UniAssetManager::UniAssetManager(std::string basePath, std::string registryFile)
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

UniAssetManager::ReturnType UniAssetManager::CheckPath(std::string path)
{
  if (std::filesystem::exists(path)) {
    return PATH_EXISTS;
  }
  return FILE_NOT_FOUND;
}

void UniAssetManager::Shutdown()
{
  for (auto& kv : m_assets) {
    kv.second->Destroy();
  }
}

UniAssetManager::ReturnType UniAssetManager::LoadRegistry()
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
    m_assets.insert({asset->m_path, asset});
  }

  return LOAD_OK;
}

// Registers an asset with a path name.
UniAssetManager::ReturnType UniAssetManager::RegisterAsset(std::string path, std::shared_ptr<UniAsset> asset, bool replace)
{
  if (m_assets.find(path) != m_assets.end() && !replace) {
    return ALREADY_CREATED;
  }

  m_assets.insert({path, asset});
  return CREATED_OK;
}

bool UniAssetManager::ImportAll()
{

  for (auto& kv : m_assets) {
    GetRegistry()->Import(kv.second->m_type, kv.second);
  }

  return true;
}

UniAssetManager::ReturnType UniAssetManager::DeleteAsset(std::string path)
{
  m_assets.erase(path);
  return DELETED_OK;
}

std::shared_ptr<UniAsset> UniAssetManager::GetAsset(std::string path)
{
  if (m_assets.find(path) == m_assets.end()) {
    return nullptr;
  }
  return m_assets.at(path);
}
