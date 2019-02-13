#include "UniMaterial.h"
#include "UniSceneRenderer.h"
#include <array>
#include "UniEngine.h"



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


void UniMaterial::SetBuffer(std::string name, std::shared_ptr<vks::Buffer> buffer) {
	m_Buffers[name] = buffer;
}

std::shared_ptr<UniSceneRenderer> UniMaterial::SceneRenderer() {
  return UniEngine::GetInstance().SceneRenderer();
}

std::shared_ptr<vks::Buffer> UniMaterial::GetBuffer(std::string buffer) {
	if(m_Buffers.find(buffer) == m_Buffers.end()) {
		m_Buffers[buffer] = std::make_shared<vks::Buffer>();
	}
	return m_Buffers.at(buffer);
}

void UniMaterial::Destroy() {

	std::cout << "Destroying base material..." << std::endl;

	auto& engine = UniEngine::GetInstance();
	auto device = engine.GetDevice();
	
	vkDestroyPipeline(device, m_Pipeline, nullptr);
	
	for(const auto& kv : m_Buffers) {
		vkDestroyBuffer(device, kv.second->buffer, nullptr);
		vkFreeMemory(device, kv.second->memory, nullptr);
	}

}
