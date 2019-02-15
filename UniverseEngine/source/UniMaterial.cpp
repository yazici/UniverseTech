#include "UniMaterial.h"
#include <array>
#include "UniEngine.h"
#include "UniSceneRenderer.h"

void UniMaterial::SetupMaterial(
    VkGraphicsPipelineCreateInfo& pipelineCreateInfo) {
  VkGraphicsPipelineCreateInfo localPCI = pipelineCreateInfo;

  auto& engine = UniEngine::GetInstance();
  auto device = engine.GetDevice();

  std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;

  shaderStages[0] =
      engine.loadShader(GetShader("vert"), VK_SHADER_STAGE_VERTEX_BIT);
  shaderStages[1] =
      engine.loadShader(GetShader("frag"), VK_SHADER_STAGE_FRAGMENT_BIT);

  localPCI.stageCount = static_cast<uint32_t>(shaderStages.size());
  localPCI.pStages = shaderStages.data();

  VK_CHECK_RESULT(vkCreateGraphicsPipelines(engine.GetDevice(),
                                            engine.GetPipelineCache(), 1,
                                            &localPCI, nullptr, &m_Pipeline));
}

void UniMaterial::LoadTexture(std::string name, std::string texturePath) {
  // Textures
  std::string texFormatSuffix;
  VkFormat texFormat;

  auto& engine = UniEngine::GetInstance();
  auto device = engine.vulkanDevice;
  auto copyQueue = engine.GetQueue();

  // Get supported compressed texture format
  if (device->features.textureCompressionBC) {
    texFormatSuffix = "_bc3_unorm";
    texFormat = VK_FORMAT_BC3_UNORM_BLOCK;
  } else if (device->features.textureCompressionASTC_LDR) {
    texFormatSuffix = "_astc_8x8_unorm";
    texFormat = VK_FORMAT_ASTC_8x8_UNORM_BLOCK;
  } else if (device->features.textureCompressionETC2) {
    texFormatSuffix = "_etc2_unorm";
    texFormat = VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
  } else {
    vks::tools::exitFatal(
        "Device does not support any compressed texture format!",
        VK_ERROR_FEATURE_NOT_PRESENT);
  }

  auto texture = std::make_shared<vks::Texture2D>();

  if (!texturePath.empty()) {
    texture->loadFromFile(
        getAssetPath() + texturePath + texFormatSuffix + ".ktx", texFormat,
        device, copyQueue);
  } else {
    std::vector<glm::vec4> buffer(4 * 4);
    for (auto& i : buffer) {
      i = glm::vec4(.6f, .6f, .6f, 1.f);
    }
    texture->fromBuffer(buffer.data(), buffer.size() * sizeof(glm::vec4),
                         VK_FORMAT_R32G32B32A32_SFLOAT, 4, 4, device, copyQueue,
                         VK_FILTER_LINEAR);
  }

  SetTexture(name, texture);

}

void UniMaterial::SetBuffer(std::string name,
                            std::shared_ptr<vks::Buffer> buffer) {
  m_Buffers[name] = buffer;
}

std::shared_ptr<vks::Buffer> UniMaterial::GetBuffer(std::string buffer) {
  if (m_Buffers.find(buffer) == m_Buffers.end()) {
    m_Buffers[buffer] = std::make_shared<vks::Buffer>();
  }
  return m_Buffers.at(buffer);
}

std::shared_ptr<vks::Texture2D> UniMaterial::GetTexture(std::string name) {
  return m_Textures.at(name);
}

void UniMaterial::SetTexture(std::string name,
                             std::shared_ptr<vks::Texture2D> texture) {
  m_Textures[name] = texture;
}

std::shared_ptr<UniSceneRenderer> UniMaterial::SceneRenderer() {
  return UniEngine::GetInstance().SceneRenderer();
}

void UniMaterial::Destroy() {
  std::cout << "Destroying base material..." << std::endl;

  auto& engine = UniEngine::GetInstance();
  auto device = engine.GetDevice();

  vkDestroyPipeline(device, m_Pipeline, nullptr);

  for (const auto& kv : m_Buffers) {
    vkDestroyBuffer(device, kv.second->buffer, nullptr);
    vkFreeMemory(device, kv.second->memory, nullptr);
  }
}
