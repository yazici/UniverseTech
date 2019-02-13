#include "ModelMaterial.h"

#include <array>
#include "../UniEngine.h"
#include "../UniSceneManager.h"
#include "../UniSceneRenderer.h"

const bool materialsAdded = [] {
  ModelMaterialFactory ModelMaterialInitializer("model", [](std::string name) {
    return std::make_unique<ModelMaterial>(name);
  });
  return true;
}();

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
  std::cout << "Calling add to command buffer for model material." << std::endl;

  vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);
  VkDeviceSize offsets[1] = {0};

  auto dynamicAlignment = UniEngine::GetInstance().getDynamicAlignment();

  auto models =
      UniEngine::GetInstance().SceneManager()->CurrentScene()->GetModels();

  auto *desc_set = UniEngine::GetInstance().SceneRenderer()->GetDescriptorSet();

  std::cout << "There are " << models.size()
            << " models for this material." << std::endl;

  for_each(models.begin(), models.end(),
           [this, &index, dynamicAlignment, cmdBuffer,
            layout, desc_set](std::shared_ptr<UniModel> model) {
             VkDeviceSize offsets[1] = {0};
             uint32_t dynamicOffset =
                 index * static_cast<uint32_t>(dynamicAlignment);
             vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                     layout, 0, 1, desc_set, 1,
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
