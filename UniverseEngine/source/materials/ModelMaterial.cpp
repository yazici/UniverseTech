#include "ModelMaterial.h"

#include <array>
#include "../UniEngine.h"
#include "../UniSceneManager.h"
#include "../UniSceneRenderer.h"

ModelMaterial::ModelMaterial(std::string name) {
  auto& engine = UniEngine::GetInstance();
  auto aPath = engine.getAssetPath();
  m_Name = name;
  SetShader("vert", aPath + "shaders/omnishader.vert.spv");
  SetShader("frag", aPath + "shaders/omnishader.frag.spv");
}

uint32_t ModelMaterial::AddToCommandBuffer(VkCommandBuffer& cmdBuffer,
                                           uint32_t index,
                                           VkPipelineLayout layout) {
  //std::cout << "Calling add to command buffer for model material." << std::endl;

  vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);
  VkDeviceSize offsets[1] = {0};

  auto dynamicAlignment = UniEngine::GetInstance().getDynamicAlignment();

  auto models = GetModels();

  //std::cout << "There are " << models.size()
  //          << " models for this material." << std::endl;

  for_each(models.begin(), models.end(),
           [this, &index, dynamicAlignment, cmdBuffer,
            layout](std::shared_ptr<UniModel> model) {
             VkDeviceSize offsets[1] = {0};
             uint32_t dynamicOffset =
                 index * static_cast<uint32_t>(dynamicAlignment);
             vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                     layout, 0, 1, &m_descriptorSet, 1,
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

void ModelMaterial::Destroy() {
  std::cout << "Destroying model material..." << std::endl;

  for (auto& tex : m_Textures) {
    tex->destroy();
  }
}

void ModelMaterial::RegisterModel(std::shared_ptr<UniModel> model) {
  m_models.push_back(model);
}

void ModelMaterial::UnRegisterModel(UniModel* model) {
  int i = 0;
  while (i < m_models.size()) {
    if (m_models[i]->GetName() == model->GetName()) {
      m_models.erase(m_models.begin() + i);
      i = 0;
    }
    i++;
  }
}

void ModelMaterial::LoadTexture(std::string name, std::string texturePath) {

  if (texturePath.empty()) {
    return;
  }

  if (name == "texture") {
    m_hasTextureMap = true;
  }
  if (name == "normal") {
    m_hasNormalMap = true;
  }
  if (name == "metallic") {
    m_hasMetallicMap = true;
  }
  if (name == "gloss") {
    m_hasGlossMap = true;
  }
  if (name == "specular") {
    m_hasSpecularMap = true;
  }
  if (name == "emissive") {
    m_hasEmissiveMap = true;
  }
  if (name == "ao") {
    m_hasAOMap = true;
  }

  UniMaterial::LoadTexture(name, texturePath);
  
}
