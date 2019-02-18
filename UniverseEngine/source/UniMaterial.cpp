#include "UniMaterial.h"
#include <array>
#include "UniEngine.h"
#include "UniSceneManager.h"
#include "UniSceneRenderer.h"

void UniMaterial::SetupMaterial(
    VkGraphicsPipelineCreateInfo& pipelineCreateInfo) {
  VkGraphicsPipelineCreateInfo localPCI = pipelineCreateInfo;

  auto& engine = UniEngine::GetInstance();
  auto device = engine.GetDevice();

  SetupDescriptorSetLayout(SceneRenderer());
  PreparePipelines(SceneRenderer(), pipelineCreateInfo);
  SetupDescriptorPool(SceneRenderer());
  SetupDescriptorSets(SceneRenderer());
}

/*
  Sets up the descriptor set layout for this material.
  This defines the textures for the material.
  For PBR, the textures are, in order:
  0: Texture Map
  1: Normal Map
  2: Roughness Map
  3: Metallic Map
  4: Specular Map
  5: Emissive Map
  6: AO Map
 */
void UniMaterial::SetupDescriptorSetLayout(
    std::shared_ptr<UniSceneRenderer> renderer) {
  // Deferred shading layout
  m_setLayoutBindings = {
      // Binding 0 : model texture map
      vks::initializers::descriptorSetLayoutBinding(
          VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
          VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0),
      // Binding 1 : model normal map
      vks::initializers::descriptorSetLayoutBinding(
          VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
          VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 1),
      // Binding 2 : model roughness map
      vks::initializers::descriptorSetLayoutBinding(
          VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
          VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 2),
      // Binding 3 : model metallic map
      vks::initializers::descriptorSetLayoutBinding(
          VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
          VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 3),
      // Binding 4 : model specular map
      vks::initializers::descriptorSetLayoutBinding(
          VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
          VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 4),
      // Binding 5 : model emissive map
      vks::initializers::descriptorSetLayoutBinding(
          VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
          VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 5),
      // Binding 6 : model ao map
      vks::initializers::descriptorSetLayoutBinding(
          VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
          VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 6),
  };

  auto device = UniEngine::GetInstance().GetDevice();

  VkDescriptorSetLayoutCreateInfo descriptorLayout =
      vks::initializers::descriptorSetLayoutCreateInfo(
          m_setLayoutBindings.data(),
          static_cast<uint32_t>(m_setLayoutBindings.size()));

  VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorLayout,
                                              nullptr, &m_descriptorSetLayout));

  std::vector<VkDescriptorSetLayout> dsLayouts = {renderer->GetDescriptorSetLayout(), m_descriptorSetLayout};

  VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo =
      vks::initializers::pipelineLayoutCreateInfo(
          dsLayouts.data(), static_cast<uint32_t>(dsLayouts.size()));

  std::vector<VkPushConstantRange> pushRanges = {
      vks::initializers::pushConstantRange(
          VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
          renderer->GetPushConstantSize() + sizeof(m_MaterialProperties), 0),
  };

  pPipelineLayoutCreateInfo.pushConstantRangeCount =
      static_cast<uint32_t>(pushRanges.size());
  pPipelineLayoutCreateInfo.pPushConstantRanges = pushRanges.data();

  VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pPipelineLayoutCreateInfo,
                                         nullptr, &m_pipelineLayout));
}

void UniMaterial::PreparePipelines(
    std::shared_ptr<UniSceneRenderer> renderer,
    VkGraphicsPipelineCreateInfo& pipelineCreateInfo) {
  VkGraphicsPipelineCreateInfo localPCI = pipelineCreateInfo;
  auto& engine = UniEngine::GetInstance();
  auto device = engine.GetDevice();

  std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;

  localPCI.layout = m_pipelineLayout;

  shaderStages[0] =
      engine.loadShader(GetShader("vert"), VK_SHADER_STAGE_VERTEX_BIT);
  shaderStages[1] =
      engine.loadShader(GetShader("frag"), VK_SHADER_STAGE_FRAGMENT_BIT);

  localPCI.stageCount = static_cast<uint32_t>(shaderStages.size());
  localPCI.pStages = shaderStages.data();

  VK_CHECK_RESULT(vkCreateGraphicsPipelines(engine.GetDevice(),
                                            engine.GetPipelineCache(), 1,
                                            &localPCI, nullptr, &m_pipeline));
}

void UniMaterial::SetupDescriptorPool(
    std::shared_ptr<UniSceneRenderer> renderer) {
  auto& engine = UniEngine::GetInstance();
  auto device = engine.GetDevice();
  auto& drawCmdBuffers = engine.GetCommandBuffers();

  auto modelCount = static_cast<uint32_t>(std::max((int)m_models.size(), 1));

  auto drawCmdBufferCount = static_cast<uint32_t>(drawCmdBuffers.size());

  std::vector<VkDescriptorPoolSize> poolSizes = {
      vks::initializers::descriptorPoolSize(
          VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
          static_cast<uint32_t>(m_setLayoutBindings.size()) * modelCount *
              drawCmdBufferCount),
      vks::initializers::descriptorPoolSize(
          VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
          modelCount * drawCmdBufferCount)};

  VkDescriptorPoolCreateInfo descriptorPoolInfo =
      vks::initializers::descriptorPoolCreateInfo(
          static_cast<uint32_t>(poolSizes.size()), poolSizes.data(),
          static_cast<uint32_t>(drawCmdBufferCount));

  VK_CHECK_RESULT(vkCreateDescriptorPool(device, &descriptorPoolInfo, nullptr,
                                         &m_descriptorPool));
}

void UniMaterial::SetupDescriptorSets(
    std::shared_ptr<UniSceneRenderer> renderer) {
  auto device = UniEngine::GetInstance().GetDevice();

  VkDescriptorSetAllocateInfo allocInfo =
      vks::initializers::descriptorSetAllocateInfo(m_descriptorPool,
                                                   &m_descriptorSetLayout, 1);

  VK_CHECK_RESULT(
      vkAllocateDescriptorSets(device, &allocInfo, &m_descriptorSet));

  m_writeDescriptorSets = {
      // Binding 0: Texture map
      vks::initializers::writeDescriptorSet(
          m_descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0,
          &GetTexture("texture")->descriptor),
      // Binding 1: Normal map
      vks::initializers::writeDescriptorSet(
          m_descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
          &GetTexture("normal")->descriptor),
      // Binding 2: Roughness map
      vks::initializers::writeDescriptorSet(
          m_descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2,
          &GetTexture("roughness")->descriptor),
      // Binding 3: Metallic map
      vks::initializers::writeDescriptorSet(
          m_descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3,
          &GetTexture("metallic")->descriptor),
      // Binding 4: Specular map
      vks::initializers::writeDescriptorSet(
          m_descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4,
          &GetTexture("specular")->descriptor),
      // Binding 5: Emissive map
      vks::initializers::writeDescriptorSet(
          m_descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 5,
          &GetTexture("emissive")->descriptor),
      // Binding 6: AO map
      vks::initializers::writeDescriptorSet(
          m_descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 6,
          &GetTexture("ao")->descriptor),
  };

  vkUpdateDescriptorSets(device,
                         static_cast<uint32_t>(m_writeDescriptorSets.size()),
                         m_writeDescriptorSets.data(), 0, nullptr);
}

uint32_t UniMaterial::AddToCommandBuffer(VkCommandBuffer& cmdBuffer,
                                         uint32_t index) {
  // std::cout << "Calling add to command buffer for model material." <<
  // std::endl;

  vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
  VkDeviceSize offsets[1] = {0};

  auto dynamicAlignment = UniEngine::GetInstance().getDynamicAlignment();

  auto models = m_models;

  auto renderer = SceneRenderer();

  std::vector<VkDescriptorSet> dSets = {renderer->GetDescriptorSet(),
                                        m_descriptorSet};

  vkCmdPushConstants(cmdBuffer, m_pipelineLayout,
                     VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT,
                     0, renderer->GetPushConstantSize(),
                     &renderer->GetPushConstants());
  vkCmdPushConstants(cmdBuffer, m_pipelineLayout,
                     VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT,
                     renderer->GetPushConstantSize(),
                     sizeof(m_MaterialProperties), &m_MaterialProperties);

  for_each(models.begin(), models.end(),
           [this, &index, dynamicAlignment, cmdBuffer,
            dSets](std::shared_ptr<UniModel> model) {
             VkDeviceSize offsets[1] = {0};
             uint32_t dynamicOffset =
                 index * static_cast<uint32_t>(dynamicAlignment);
             vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                     m_pipelineLayout, 0,
                                     static_cast<uint32_t>(dSets.size()),
                                     dSets.data(),
                                     1,
                                     &dynamicOffset);
             vkCmdBindVertexBuffers(cmdBuffer, VERTEX_BUFFER_BIND_ID, 1,
                                    &model->m_Model.vertices.buffer, offsets);
             vkCmdBindIndexBuffer(cmdBuffer, model->m_Model.indices.buffer, 0,
                                  VK_INDEX_TYPE_UINT32);
             vkCmdDrawIndexed(cmdBuffer, model->m_Model.indexCount, 1, 0, 0, 0);
             index++;
           });

  return index;
}

void UniMaterial::LoadTexture(std::string name, std::string texturePath) {
  // Textures
  //  std::string texFormatSuffix;
  VkFormat texFormat = VK_FORMAT_R8G8B8A8_UNORM;

  auto& engine = UniEngine::GetInstance();
  auto device = engine.vulkanDevice;
  auto copyQueue = engine.GetQueue();

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

  if (!texturePath.empty()) {
    texture->loadFromFile(getAssetPath() + texturePath, texFormat, device,
                          copyQueue);
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
  if (m_Textures.find(name) == m_Textures.end()) {
    std::cout << "No texture named " << name << ", creating empty buffer."
              << std::endl;
    LoadTexture(name, "");
  }
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

  vkDestroyPipeline(device, m_pipeline, nullptr);

  for (const auto& kv : m_Buffers) {
    vkDestroyBuffer(device, kv.second->buffer, nullptr);
    vkFreeMemory(device, kv.second->memory, nullptr);
  }
}

void UniMaterial::RegisterModel(std::shared_ptr<UniModel> model) {
  m_models.push_back(model);
}

void UniMaterial::UnRegisterModel(UniModel* model) {
  int i = 0;
  while (i < m_models.size()) {
    if (m_models[i]->GetName() == model->GetName()) {
      m_models.erase(m_models.begin() + i);
      i = 0;
    }
    i++;
  }
}