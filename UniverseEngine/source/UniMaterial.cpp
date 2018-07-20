#include "UniMaterial.h"
#include <array>
#include "UniEngine.h"


const bool materialsAdded = [] {
	MaterialFactory PlanetMaterialInitializer("planet", [](std::string name) { return std::make_unique<PlanetMaterial>(name); });
	return true;
}();


void PlanetMaterial::SetupMaterial(VkGraphicsPipelineCreateInfo& pipelineCreateInfo) {

	auto& engine = UniEngine::GetInstance();
	auto device = engine.GetDevice();


	// Binding description
	m_VertexDescription.bindingDescriptions.resize(1);
	m_VertexDescription.bindingDescriptions[0] =
		vks::initializers::vertexInputBindingDescription(
			VERTEX_BUFFER_BIND_ID,
			m_VertexLayout.stride(),
			VK_VERTEX_INPUT_RATE_VERTEX);

	// Attribute descriptions
	m_VertexDescription.attributeDescriptions.resize(1);
	// Location 0: Position
	m_VertexDescription.attributeDescriptions[0] =
		vks::initializers::vertexInputAttributeDescription(
			VERTEX_BUFFER_BIND_ID,
			0,
			VK_FORMAT_R32G32B32_SFLOAT,
			0);


	m_VertexDescription.inputState = vks::initializers::pipelineVertexInputStateCreateInfo();
	m_VertexDescription.inputState.vertexBindingDescriptionCount = static_cast<uint32_t>(m_VertexDescription.bindingDescriptions.size());
	m_VertexDescription.inputState.pVertexBindingDescriptions = m_VertexDescription.bindingDescriptions.data();
	m_VertexDescription.inputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(m_VertexDescription.attributeDescriptions.size());
	m_VertexDescription.inputState.pVertexAttributeDescriptions = m_VertexDescription.attributeDescriptions.data();


	// Deferred shading layout
	std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings =
	{
		// Binding 0 : Vertex shader uniform buffer
		vks::initializers::descriptorSetLayoutBinding(
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			0),
		// Binding 1 : This is the continent noise texture
		vks::initializers::descriptorSetLayoutBinding(
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT,
			1),
		// Binding 1 : This is the terrain color ramp texture
		vks::initializers::descriptorSetLayoutBinding(
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT,
			2)

	};

	VkDescriptorSetLayoutCreateInfo descriptorLayout =
		vks::initializers::descriptorSetLayoutCreateInfo(
			setLayoutBindings.data(),
			static_cast<uint32_t>(setLayoutBindings.size()));

	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorLayout, nullptr, &m_DescriptorSetLayout));


	VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo =
		vks::initializers::pipelineLayoutCreateInfo(
			&m_DescriptorSetLayout,
			1);

	VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pPipelineLayoutCreateInfo, nullptr, &m_PipelineLayout));


	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;

	pipelineCreateInfo.pVertexInputState = &m_VertexDescription.inputState;

	shaderStages[0] = engine.loadShader(engine.getAssetPath() + m_VertexShader, VK_SHADER_STAGE_VERTEX_BIT);
	shaderStages[1] = engine.loadShader(engine.getAssetPath() + m_FragmentShader, VK_SHADER_STAGE_FRAGMENT_BIT);
	
	pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineCreateInfo.pStages = shaderStages.data();
	pipelineCreateInfo.layout = m_PipelineLayout;

	VK_CHECK_RESULT(vkCreateGraphicsPipelines(engine.GetDevice(), engine.GetPipelineCache() , 1, &pipelineCreateInfo, nullptr, &m_Pipeline));


	std::vector<VkDescriptorPoolSize> poolSizes =
	{
		vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1),
		vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2),
	};

	VkDescriptorPoolCreateInfo descriptorPoolInfo =
		vks::initializers::descriptorPoolCreateInfo(
			static_cast<uint32_t>(poolSizes.size()),
			poolSizes.data(),
			static_cast<uint32_t>(1));

	VK_CHECK_RESULT(vkCreateDescriptorPool(device, &descriptorPoolInfo, nullptr, &m_DescriptorPool));


	// Textured quad descriptor set
	VkDescriptorSetAllocateInfo allocInfo =
		vks::initializers::descriptorSetAllocateInfo(
			m_DescriptorPool,
			&m_DescriptorSetLayout,
			1);

	VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, &m_DescriptorSet));

	std::vector<VkWriteDescriptorSet> writeDescriptorSets =
	{
		// Binding 0: Vertex shader uniform buffer
		vks::initializers::writeDescriptorSet(
			m_DescriptorSet,
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			0,
			&GetBuffer("uniform")->descriptor),
		// Binding 1: Continent texture map
		vks::initializers::writeDescriptorSet(
			m_DescriptorSet,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			1,
			&m_Textures[0]->descriptor),
		// Binding 1: Terrain color ramp texture
		vks::initializers::writeDescriptorSet(
			m_DescriptorSet,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			2,
			&m_Textures[1]->descriptor),

	};
	vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);


}

PlanetMaterial::PlanetMaterial(std::string name) {
	m_Name = name;
	m_VertexShader = "shaders/uniplanet.vert.spv";
	m_FragmentShader = "shaders/uniplanet.frag.spv";

}


void UniMaterial::SetBuffer(std::string name, std::shared_ptr<vks::Buffer> buffer) {
	m_Buffers[name] = buffer;
}

void PlanetMaterial::AddToCommandBuffer(VkCommandBuffer& cmdBuffer) {
	vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);

	VkDeviceSize offsets[1] = { 0 };

	vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1, &m_DescriptorSet, 0, nullptr);
	vkCmdBindVertexBuffers(cmdBuffer, VERTEX_BUFFER_BIND_ID, 1, &GetBuffer("vertex")->buffer, offsets);
	vkCmdBindIndexBuffer(cmdBuffer, GetBuffer("index")->buffer, 0, VK_INDEX_TYPE_UINT32);
	vkCmdDrawIndexed(cmdBuffer, m_IndexCount, 1, 0, 0, 0);

}

void PlanetMaterial::Destroy() {
	std::cout << "Destroying planet material..." << std::endl;

	for(auto& tex : m_Textures) {
		tex->destroy();
	}

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
	vkDestroyPipelineLayout(device, m_PipelineLayout, nullptr);
	vkDestroyDescriptorSetLayout(device, m_DescriptorSetLayout, nullptr);
	
	for(const auto& kv : m_Buffers) {
		vkDestroyBuffer(device, kv.second->buffer, nullptr);
		vkFreeMemory(device, kv.second->memory, nullptr);
	}

}
