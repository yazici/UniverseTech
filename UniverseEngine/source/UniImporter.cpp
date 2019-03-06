#include "UniImporter.h"
#include "UniAsset.h"
#include "UniEngine.h"
#include "UniAssetManager.h"
#include "UniSceneRenderer.h"
#include "UniAudioEngine.h"

using namespace uni::import;


std::shared_ptr<UniAsset> UniTexture2DImporter::Import(std::shared_ptr<UniAsset> asset)
{
  auto textureAsset = std::dynamic_pointer_cast<UniAssetTexture2D>(asset);
  if (textureAsset == nullptr) {
    vks::tools::exitFatal("Unable to import asset as texture2d.", -1);
  }

  // Textures
//  std::string texFormatSuffix;
  VkFormat texFormat = VK_FORMAT_R8G8B8A8_UNORM;

  auto engine = UniEngine::GetInstance();
  auto device = engine->vulkanDevice;
  auto copyQueue = engine->GetQueue();

  //// Get supported compressed texture format
  // if (device->features.textureCompressionBC) {
  //  texFormatSuffix = "_bc3_unorm";
  //  texFormat = VK_FORMAT_BC3_UNORM_BLOCK;
  //} else if (device->features.textureCompressionASTC_LDR) {
  //  texFormatSuffix = "_astc_8x8_unorm";
  //  texFormat = VK_FORMAT_ASTC_8x8_UNORM_BLOCK;
  //} else if (device->features.textureCompressionETC2) {
  //  texFormatSuffix = "_etc2_unorm";
  //  texFormat = VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
  //} else {
  //  vks::tools::exitFatal(
  //      "Device does not support any compressed texture format!",
  //      VK_ERROR_FEATURE_NOT_PRESENT);

  //}

  auto texture = std::make_shared<vks::Texture2D>();

  if (!textureAsset->m_sourceFile.empty()) {
    texture->loadFromFile(textureAsset->m_sourceFile, texFormat, device,
      copyQueue);

  }
  else {
    std::vector<glm::vec4> buffer(4 * 4);
    for (auto& i : buffer) {
      i = glm::vec4(.6f, .6f, .6f, 1.f);
    }
    texture->fromBuffer(buffer.data(), buffer.size() * sizeof(glm::vec4),
      VK_FORMAT_R32G32B32A32_SFLOAT, 4, 4, device, copyQueue,
      VK_FILTER_LINEAR);
  }

  textureAsset->m_texture = texture;

  return textureAsset;

}

std::shared_ptr<UniAsset> UniTexture2DImporter::LoadAsset(json data)
{
  return CreateAsset<UniAssetTexture2D>(data);
}


std::shared_ptr<UniAsset> UniModelImporter::Import(std::shared_ptr<UniAsset> asset)
{
  auto modelAsset = std::dynamic_pointer_cast<UniAssetModel>(asset);
  if (modelAsset == nullptr) {
    vks::tools::exitFatal("Unable to import asset as model.", -1);
  }


  std::vector<std::string> materials = asset->m_settings.at("materials");


  std::cout << "Creating model path: " << asset->m_path << std::endl;


  glm::vec3 createScale = { asset->m_settings.at("createScale")[0], asset->m_settings.at("createScale")[1],
                           asset->m_settings.at("createScale")[2] };
  glm::vec2 createUVScale = { asset->m_settings.at("createUVScale")[0],
                             asset->m_settings.at("createUVScale")[1] };
  glm::vec3 createOffset = glm::vec3(0);

  if (asset->m_settings.find("createOffset") != asset->m_settings.end()) {
    createOffset = { asset->m_settings.at("createOffset")[0], asset->m_settings.at("createOffset")[1],
                    asset->m_settings.at("createOffset")[2] };
  }

  auto model = std::make_shared<uni::Model>();

  uni::ModelCreateInfo mci(createScale, createUVScale, createOffset);

  auto testFlags = aiProcess_Triangulate | aiProcess_CalcTangentSpace |
    aiProcess_FindDegenerates | aiProcess_OptimizeMeshes |
    aiProcess_FindInvalidData | aiProcess_GenSmoothNormals;

  auto engine = UniEngine::GetInstance();
  auto renderer = engine->SceneRenderer();

  model->loadFromFile(asset->m_sourceFile, renderer->GetVertexLayout(),
    &mci, engine->vulkanDevice, engine->GetQueue(), materials,
    testFlags);

  modelAsset->m_model = model;
  modelAsset->m_materials = materials;


  return modelAsset;

}

std::shared_ptr<UniAsset> UniModelImporter::LoadAsset(json data)
{
  return CreateAsset<UniAssetModel>(data);
}

std::shared_ptr<UniAsset> MaterialImporter::Import(std::shared_ptr<UniAsset> asset) {

  auto materialAsset = std::dynamic_pointer_cast<UniAssetMaterial>(asset);
  if (materialAsset == nullptr) {
    vks::tools::exitFatal("Unable to import asset as material.", -1);
  }

  std::cout << "Loading model material: " << asset->m_path << std::endl;
  // 	m_Name = jsonData["name"];
  std::string materialID = asset->m_path;

  auto so = asset->m_settings;

  std::string defaultPath = "/textures/default";
  bool useTexture = false;
  bool useNormal = false;
  bool useRoughness = false;
  bool useMetallic = false;
  bool useSpecular = false;
  bool useEmissive = false;
  bool useAO = false;

  std::string texturePath = defaultPath;
  if (so.find("textureMap") != so.end()) {
    useTexture = true;
    texturePath = so.at("textureMap");
  }

  std::string normalPath = defaultPath;
  if (so.find("normalMap") != so.end()){
    normalPath = so.at("normalMap");
    useNormal = true;
  }

  std::string metallicPath = defaultPath;
  if (so.find("metallicMap") != so.end()) {
    useMetallic = true;
    metallicPath = so.at("metallicMap");
  }
    

  std::string roughnessPath = defaultPath;
  if (so.find("roughnessMap") != so.end()) {
    useRoughness = true;
    roughnessPath = so.at("roughnessMap");
  }
    

  std::string specularPath = defaultPath;
  if (so.find("specularMap") != so.end()) {
    useSpecular = true;
    specularPath = so.at("specularMap");
  }

  std::string emissivePath = defaultPath;
  if (so.find("emissiveMap") != so.end()) {
    useEmissive = true;
    emissivePath = so.at("emissiveMap");
  }

  std::string aoPath = defaultPath;
  if (so.find("aoMap") != so.end()) {
    useAO = true;
    aoPath = so.at("aoMap");
  }

  glm::vec4 baseColour = glm::vec4(1.f);
  if (so.find("colour") != so.end()) {
    auto bc = so.at("colour");
    baseColour = glm::vec4(bc[0], bc[1], bc[2], bc[3]);
  }

  glm::vec4 emissiveColour = glm::vec4(0.f);
  if (so.find("emissive") != so.end()) {
    auto bc = so.at("emissive");
    emissiveColour = glm::vec4(bc[0], bc[1], bc[2], bc[3]);
  }

  float specular = 0.04f;
  float metallic = 0.f;
  float roughness = 1.f;

  if (so.find("specular") != so.end()) {
    specular = so.at("specular");
  }

  if (so.find("metallic") != so.end()) {
    metallic = so.at("metallic");
  }

  if (so.find("roughness") != so.end()) {
    roughness = so.at("roughness");
  }

  if (materialID == "/materials/spaceship01/material1") {
    std::cout << "Importing the material asset!" << std::endl;
  }

  auto material = std::make_shared<ModelMaterial>(materialID);
  material->LoadTexture("texture", texturePath);
  material->LoadTexture("normal", normalPath);
  material->LoadTexture("metallic", metallicPath);
  material->LoadTexture("specular", specularPath);
  material->LoadTexture("roughness", roughnessPath);
  material->LoadTexture("emissive", emissivePath);
  material->LoadTexture("ao", aoPath);
  material->SetBaseColour(baseColour);
  material->SetEmissiveColour(emissiveColour);
  material->SetRoughness(roughness);
  material->SetMetallic(metallic);
  material->SetSpecular(specular);
  material->SetUseTexture(useTexture);
  material->SetUseNormal(useNormal);
  material->SetUseRoughness(useRoughness);
  material->SetUseMetallic(useMetallic);
  material->SetUseSpecular(useSpecular);
  material->SetUseEmissive(useEmissive);
  material->SetUseAO(useAO);

  std::cout << "There are now " << material->m_Textures.size() << " textures and " << material->m_TexturePaths.size() << " paths registered with " << material->GetName() << std::endl;

  materialAsset->m_material = material;

  return materialAsset;

}

std::shared_ptr<UniAsset> MaterialImporter::LoadAsset(json data)
{
  return CreateAsset<UniAssetMaterial>(data);
}

std::shared_ptr<UniAsset> AudioImporter::Import(std::shared_ptr<UniAsset> asset)
{
  auto audioAsset = std::dynamic_pointer_cast<UniAssetAudio>(asset);
  if (audioAsset == nullptr) {
    //vks::tools::exitFatal("Unable to import asset as audio.", -1);
    throw std::runtime_error("Unable to import asset as audio.");
  }

  auto is3d = static_cast<bool>(audioAsset->m_settings.at("3d"));
  auto isLooping = static_cast<bool>(audioAsset->m_settings.at("looping"));
  bool isStreaming = false;

  try {
    isStreaming = static_cast<bool>(audioAsset->m_settings.at("streaming"));
  }
  catch (json::out_of_range err) {}

  UniEngine::GetInstance()->AudioManager()->LoadSound(
    audioAsset->m_sourceFile, is3d, isLooping, false);

  return audioAsset;
}

std::shared_ptr<UniAsset> AudioImporter::LoadAsset(json data)
{
  return CreateAsset<UniAssetAudio>(data);
}


const bool factoriesAdded = [] {
  UniAssetManager::RegisterImporter("texture2d", std::make_shared<UniTexture2DImporter>());
  UniAssetManager::RegisterImporter("model", std::make_shared<UniModelImporter>());
  UniAssetManager::RegisterImporter("model material", std::make_shared<MaterialImporter>());
  UniAssetManager::RegisterImporter("audio", std::make_shared<AudioImporter>());
  return true;
}();

