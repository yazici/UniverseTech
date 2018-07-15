#include "UniMaterial.h"
#include <array>
#include "UniEngine.h"


const bool materialsAdded = [] {
	MaterialFactory PlanetMaterialInitializer("planet", [](std::string name) { return std::make_unique<PlanetMaterial>(name); });
	return true;
}();


void PlanetMaterial::SetupPipeline(VkGraphicsPipelineCreateInfo& pipelineCreateInfo) {
	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;

	auto& engine = UniEngine::GetInstance();
	
	shaderStages[0] = engine.loadShader(engine.getAssetPath() + m_VertexShader, VK_SHADER_STAGE_VERTEX_BIT);
	shaderStages[1] = engine.loadShader(engine.getAssetPath() + m_FragmentShader, VK_SHADER_STAGE_FRAGMENT_BIT);
	
	pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineCreateInfo.pStages = shaderStages.data();

	VK_CHECK_RESULT(vkCreateGraphicsPipelines(engine.GetDevice(), engine.GetPipelineCache() , 1, &pipelineCreateInfo, nullptr, &m_Pipeline));
}

PlanetMaterial::PlanetMaterial(std::string name) {
	m_Name = name;
	m_VertexShader = "shaders/uniplanet.vert.spv";
	m_FragmentShader = "shaders/uniplanet.frag.spv";
}

