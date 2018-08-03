#include "UniMaterial.h"
#include <array>
#include "UniEngine.h"


const bool materialsAdded = [] {
	MaterialFactory PlanetMaterialInitializer("planet", [](std::string name, bool hasOcean) { return std::make_unique<PlanetMaterial>(name, hasOcean); });
	return true;
}();


void PlanetMaterial::SetupMaterial(VkGraphicsPipelineCreateInfo& pipelineCreateInfo) {

	VkGraphicsPipelineCreateInfo localPCI = pipelineCreateInfo;

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState =
		vks::initializers::pipelineInputAssemblyStateCreateInfo(
			VK_PRIMITIVE_TOPOLOGY_PATCH_LIST,
			0,
			VK_FALSE);

	// we use QUADS in tessellation
	VkPipelineTessellationStateCreateInfo tessellationState =
		vks::initializers::pipelineTessellationStateCreateInfo(4);

	localPCI.pInputAssemblyState = &inputAssemblyState;
	localPCI.pTessellationState = &tessellationState;

	assert(localPCI.pTessellationState != pipelineCreateInfo.pTessellationState);

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
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
			0),
		// Binding 1 : This is the continent noise texture
		vks::initializers::descriptorSetLayoutBinding(
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
			1),
		// Binding 1 : This is the terrain color ramp texture
		vks::initializers::descriptorSetLayoutBinding(
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT,
			2),
		// Binding 3 : Noise layer storage buffer
		vks::initializers::descriptorSetLayoutBinding(
			VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
			3),

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

	VkPushConstantRange pCR = vks::initializers::pushConstantRange(
		VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
		sizeof(m_PushConstants),
		0
	);

	pPipelineLayoutCreateInfo.pushConstantRangeCount = 1;
	pPipelineLayoutCreateInfo.pPushConstantRanges = &pCR;
	
	VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pPipelineLayoutCreateInfo, nullptr, &m_PipelineLayout));


	std::array<VkPipelineShaderStageCreateInfo, 4> shaderStages;

	localPCI.pVertexInputState = &m_VertexDescription.inputState;

	shaderStages[0] = engine.loadShader(GetShader("vert"), VK_SHADER_STAGE_VERTEX_BIT);
	shaderStages[1] = engine.loadShader(GetShader("frag"), VK_SHADER_STAGE_FRAGMENT_BIT);
	shaderStages[2] = engine.loadShader(GetShader("tesc"), VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
	shaderStages[3] = engine.loadShader(GetShader("tese"), VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);

	
	// Each shader constant of a shader stage corresponds to one map entry
	std::array<VkSpecializationMapEntry, 1> specializationMapEntries;
	// Shader bindings based on specialization constants are marked by the new "constant_id" layout qualifier:
	//	layout (constant_id = 0) const bool DISPLACEMENT_USED = true;


	// Map entry for the lighting model to be used by the fragment shader
	specializationMapEntries[0].constantID = 0;
	specializationMapEntries[0].size = sizeof(m_SpecializationData.isDisplaced);
	specializationMapEntries[0].offset = 0;

	// Prepare specialization info block for the shader stage
	VkSpecializationInfo specializationInfo{};
	specializationInfo.dataSize = sizeof(m_SpecializationData);
	specializationInfo.mapEntryCount = static_cast<uint32_t>(specializationMapEntries.size());
	specializationInfo.pMapEntries = specializationMapEntries.data();
	specializationInfo.pData = &m_SpecializationData;

	shaderStages[2].pSpecializationInfo = &specializationInfo;
	shaderStages[3].pSpecializationInfo = &specializationInfo;

	localPCI.stageCount = static_cast<uint32_t>(shaderStages.size());
	localPCI.pStages = shaderStages.data();
	localPCI.layout = m_PipelineLayout;

	VK_CHECK_RESULT(vkCreateGraphicsPipelines(engine.GetDevice(), engine.GetPipelineCache(), 1, &localPCI, nullptr, &m_Pipeline));

	// OCEAN PIPELINE
	if(m_RenderOcean) {

		/*inputAssemblyState =
			vks::initializers::pipelineInputAssemblyStateCreateInfo(
				VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
				0,
				VK_FALSE);
		localPCI.pInputAssemblyState = &inputAssemblyState;
		localPCI.pTessellationState = nullptr;

		VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pPipelineLayoutCreateInfo, nullptr, &m_OceanPipelineLayout));


		localPCI.pVertexInputState = &m_VertexDescription.inputState;*/

		shaderStages[0] = engine.loadShader(GetShader("oceanvert"), VK_SHADER_STAGE_VERTEX_BIT);
		shaderStages[1] = engine.loadShader(GetShader("oceanfrag"), VK_SHADER_STAGE_FRAGMENT_BIT);
		shaderStages[2] = engine.loadShader(GetShader("oceantesc"), VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
		shaderStages[3] = engine.loadShader(GetShader("oceantese"), VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);

		shaderStages[2].pSpecializationInfo = &specializationInfo;
		shaderStages[3].pSpecializationInfo = &specializationInfo;

		m_SpecializationData.isDisplaced = false;

		/*localPCI.layout = m_OceanPipelineLayout;*/

		VK_CHECK_RESULT(vkCreateGraphicsPipelines(engine.GetDevice(), engine.GetPipelineCache(), 1, &localPCI, nullptr, &m_OceanPipeline));

	}


	std::vector<VkDescriptorPoolSize> poolSizes =
	{
		vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1),
		vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2),
		vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1),
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
		// Binding 2: Terrain color ramp texture
		vks::initializers::writeDescriptorSet(
			m_DescriptorSet,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			2,
			&m_Textures[1]->descriptor),
		// Binding 3: Noise layer storage buffer
		vks::initializers::writeDescriptorSet(
			m_DescriptorSet,
			VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			3,
			&GetBuffer("noiselayers")->descriptor),

	};
	vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);


}

PlanetMaterial::PlanetMaterial(std::string name, bool hasOcean) {
	auto& engine = UniEngine::GetInstance();
	auto aPath = engine.getAssetPath();
	m_Name = name;
	m_RenderOcean = hasOcean;
	SetShader("vert", aPath + "shaders/uniplanet.vert.spv");
	SetShader("frag", aPath + "shaders/uniplanet.frag.spv");
	SetShader("tesc", aPath + "shaders/uniplanet.tesc.spv");
	SetShader("tese", aPath + "shaders/uniplanet.tese.spv");

	if(m_RenderOcean) {
		// todo: ocean needs to be tessellated because it's too square!
		SetShader("oceanvert", aPath + "shaders/uniocean.vert.spv");
		SetShader("oceanfrag", aPath + "shaders/uniocean.frag.spv");
		SetShader("oceantesc", aPath + "shaders/uniocean.tesc.spv");
		SetShader("oceantese", aPath + "shaders/uniocean.tese.spv");
	}

}


void UniMaterial::SetBuffer(std::string name, std::shared_ptr<vks::Buffer> buffer) {
	m_Buffers[name] = buffer;
}

void PlanetMaterial::AddToCommandBuffer(VkCommandBuffer& cmdBuffer) {
	vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);

	VkDeviceSize offsets[1] = { 0 };

	// surface
	vkCmdPushConstants(
		cmdBuffer,
		m_PipelineLayout,
		VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
		0,
		sizeof(m_PushConstants),
		&m_PushConstants
	);

	vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1, &m_DescriptorSet, 0, nullptr);
	vkCmdBindVertexBuffers(cmdBuffer, VERTEX_BUFFER_BIND_ID, 1, &GetBuffer("vertex")->buffer, offsets);
	vkCmdBindIndexBuffer(cmdBuffer, GetBuffer("index")->buffer, 0, VK_INDEX_TYPE_UINT32);
	vkCmdDrawIndexed(cmdBuffer, m_IndexCount, 1, 0, 0, 0);


	// ocean

	if(m_RenderOcean) {
		vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_OceanPipeline);

		vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1, &m_DescriptorSet, 0, nullptr);
		vkCmdBindVertexBuffers(cmdBuffer, VERTEX_BUFFER_BIND_ID, 1, &GetBuffer("oceanvertex")->buffer, offsets);
		vkCmdBindIndexBuffer(cmdBuffer, GetBuffer("index")->buffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(cmdBuffer, m_OceanIndexCount, 1, 0, 0, 0);

	}


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
	vkDestroyPipeline(device, m_OceanPipeline, nullptr);
	vkDestroyPipelineLayout(device, m_OceanPipelineLayout, nullptr);
	vkDestroyDescriptorSetLayout(device, m_DescriptorSetLayout, nullptr);
	
	for(const auto& kv : m_Buffers) {
		vkDestroyBuffer(device, kv.second->buffer, nullptr);
		vkFreeMemory(device, kv.second->memory, nullptr);
	}

}
