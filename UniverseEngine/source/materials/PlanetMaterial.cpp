#include "PlanetMaterial.h"

#include <array>
#include "../UniEngine.h"
#include "../UniSceneRenderer.h"

const bool materialsAdded = [] {
  PlanetMaterialFactory PlanetMaterialInitializer(
      "planet", [](std::string name, bool hasOcean) {
        return std::make_unique<PlanetMaterial>(name, hasOcean);
      });
  return true;
}();


PlanetMaterial::PlanetMaterial(std::string name, bool hasOcean) {
  auto& engine = UniEngine::GetInstance();
  auto aPath = engine.getAssetPath();
  m_Name = name;
  m_RenderOcean = false;
  SetShader("vert", aPath + "shaders/unirayplanet.vert.spv");
  SetShader("frag", aPath + "shaders/unirayplanet.frag.spv");

  // if(m_RenderOcean) {
  //	// todo: ocean needs to be tessellated because it's too square!
  //	SetShader("oceanvert", aPath + "shaders/uniocean.vert.spv");
  //	SetShader("oceanfrag", aPath + "shaders/uniocean.frag.spv");
  //	SetShader("oceantesc", aPath + "shaders/uniocean.tesc.spv");
  //	SetShader("oceantese", aPath + "shaders/uniocean.tese.spv");
  //}
}

uint32_t PlanetMaterial::AddToCommandBuffer(VkCommandBuffer& cmdBuffer, uint32_t index, VkPipelineLayout layout){
  vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);
  vkCmdDraw(cmdBuffer, 3, 1, 0, 0);

  return index;
}

void PlanetMaterial::Destroy() {
  std::cout << "Destroying planet material..." << std::endl;

  for (auto& tex : m_Textures) {
    tex->destroy();
  }

}
