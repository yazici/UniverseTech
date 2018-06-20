#include "Patch.hpp"

#include "Components.h"
// #include "../Components/CameraComponent.hpp"
#include "UniEngine.h"
#include "UniBody.h"
#include "UniFrustum.hpp"
#include "Triangulator.hpp"
#include "vks/VulkanTools.h"


Patch::Patch(uint16_t levels)
	:m_Levels(levels) {}

void Patch::Init() {

	auto device = UniEngine::GetInstance().vulkanDevice;

	auto bufferSize = 256 * sizeof(PatchInstance); // initially allocate a buffer big enough for 256 patch instances

	VK_CHECK_RESULT(device->createBuffer(
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
		&instanceBuffer, bufferSize));

	instanceBuffer.map();

	// Attribute descriptions
	vertexDescription.attributeDescriptions.resize(6);
	uint32_t offset = 0;
	// Location 0: Pos
	vertexDescription.attributeDescriptions[0] =
		vks::initializers::vertexInputAttributeDescription(
			VERTEX_BUFFER_BIND_ID,
			0,
			VK_FORMAT_R32G32_SFLOAT,
			offset);
	offset += sizeof(glm::vec2);
	// Location 1: Morph
	vertexDescription.attributeDescriptions[1] =
		vks::initializers::vertexInputAttributeDescription(
			VERTEX_BUFFER_BIND_ID,
			1,
			VK_FORMAT_R32G32_SFLOAT,
			offset);
	offset += sizeof(glm::vec2);
	// Location 2: Level
	vertexDescription.attributeDescriptions[2] =
		vks::initializers::vertexInputAttributeDescription(
			VERTEX_BUFFER_BIND_ID,
			2,
			VK_FORMAT_R32_UINT,
			offset);
	offset += sizeof(uint32_t);
	// Location 3: A
	vertexDescription.attributeDescriptions[3] =
		vks::initializers::vertexInputAttributeDescription(
			VERTEX_BUFFER_BIND_ID,
			3,
			VK_FORMAT_R32G32B32_SFLOAT,
			offset);
	offset += sizeof(glm::vec3);
	// Location 4: R
	vertexDescription.attributeDescriptions[4] =
		vks::initializers::vertexInputAttributeDescription(
			VERTEX_BUFFER_BIND_ID,
			3,
			VK_FORMAT_R32G32B32_SFLOAT,
			offset);
	offset += sizeof(glm::vec3);
	// Location 5: S
	vertexDescription.attributeDescriptions[5] =
		vks::initializers::vertexInputAttributeDescription(
			VERTEX_BUFFER_BIND_ID,
			3,
			VK_FORMAT_R32G32B32_SFLOAT,
			offset);

	vertexDescription.inputState = vks::initializers::pipelineVertexInputStateCreateInfo();
	vertexDescription.inputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexDescription.attributeDescriptions.size());
	vertexDescription.inputState.pVertexAttributeDescriptions = vertexDescription.attributeDescriptions.data();

	GenerateGeometry(m_Levels);
}

void Patch::GenerateGeometry(uint16_t levels) {
	//clear
	m_Vertices.clear();
	m_Indices.clear();
	//Generate
	m_Levels = levels;
	m_RC = 1 + (uint32_t)pow(2, (uint32_t)m_Levels);

	float delta = 1 / (float)(m_RC - 1);

	uint32_t rowIdx = 0;
	uint32_t nextIdx = 0;
	for(uint32_t row = 0; row < m_RC; row++) {
		uint32_t numCols = m_RC - row;
		nextIdx += numCols;
		for(uint32_t column = 0; column < numCols; column++) {
			//calc position
			glm::vec2 pos = glm::vec2(column / (float)(m_RC - 1), row / (float)(m_RC - 1));
			//calc morph
			glm::vec2 morph = glm::vec2(0, 0);
			if(row % 2 == 0) {
				if(column % 2 == 1) morph = glm::vec2(-delta, 0);
			} else {
				if(column % 2 == 0) morph = glm::vec2(0, delta);
				else morph = glm::vec2(delta, -delta);
			}
			//create vertex
			m_Vertices.emplace_back(pos, morph);
			//calc index
			if(row < m_RC - 1 && column < numCols - 1) {
				m_Indices.push_back(rowIdx + column);
				m_Indices.push_back(nextIdx + column);
				m_Indices.push_back(1 + rowIdx + column);
				if(column < numCols - 2) {
					m_Indices.push_back(nextIdx + column);
					m_Indices.push_back(1 + nextIdx + column);
					m_Indices.push_back(1 + rowIdx + column);
				}
			}
		}
		rowIdx = nextIdx;
	}

	uint32_t vertexBufferSize = static_cast<uint32_t>(m_Vertices.size() * sizeof(PatchVertex));
	uint32_t indexBufferSize = static_cast<uint32_t>(m_Indices.size() * sizeof(uint32_t));

	vks::Buffer vertexStaging, indexStaging;

	auto device = UniEngine::GetInstance().vulkanDevice;

	// Vertex buffer
	VK_CHECK_RESULT(device->createBuffer(
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&vertexStaging,
		vertexBufferSize,
		m_Vertices.data()));

	// Index buffer
	VK_CHECK_RESULT(device->createBuffer(
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&indexStaging,
		indexBufferSize,
		m_Indices.data()));

	// Create device local target buffers
	// Vertex buffer
	VK_CHECK_RESULT(device->createBuffer(
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		&vertexBuffer,
		vertexBufferSize));

	// Index buffer
	VK_CHECK_RESULT(device->createBuffer(
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		&indexBuffer,
		indexBufferSize));

	// Copy from staging buffers
	VkCommandBuffer copyCmd = device->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

	VkBufferCopy copyRegion{};

	copyRegion.size = vertexBuffer.size;
	vkCmdCopyBuffer(copyCmd, vertexStaging.buffer, vertexBuffer.buffer, 1, &copyRegion);

	copyRegion.size = indexBuffer.size;
	vkCmdCopyBuffer(copyCmd, indexStaging.buffer, indexBuffer.buffer, 1, &copyRegion);

	device->flushCommandBuffer(copyCmd, UniEngine::GetInstance().GetQueue());

	// Destroy staging resources
	vkDestroyBuffer(device->logicalDevice, vertexStaging.buffer, nullptr);
	vkFreeMemory(device->logicalDevice, vertexStaging.memory, nullptr);
	vkDestroyBuffer(device->logicalDevice, indexStaging.buffer, nullptr);
	vkFreeMemory(device->logicalDevice, indexStaging.memory, nullptr);

}

void Patch::BindInstances(std::vector<PatchInstance> &instances) {
	//update buffer
	m_NumInstances = (uint32_t)instances.size();
	
	auto device = UniEngine::GetInstance().vulkanDevice;
	auto neededSize = m_NumInstances * sizeof(PatchInstance);
	if(neededSize > instanceBuffer.size) {
		instanceBuffer.destroy();
		VK_CHECK_RESULT(device->createBuffer(
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
			&instanceBuffer, neededSize));
		instanceBuffer.map();
	}

	instanceBuffer.copyTo(instances.data(), neededSize);
	
}

void Patch::UploadDistanceLUT(std::vector<float> &distances) {
	for(size_t i = 0; i < distances.size(); i++) {
		uniformBufferData.distanceLut[i] = distances[i];
	}
}

void Patch::Draw() {
	
	//// Pass transformations to the shader
	uniformBufferData.model = m_pPlanet->GetTransform()->GetModelMat();
	uniformBufferData.viewProj = UniEngine::GetInstance().camera.matrices.perspective * UniEngine::GetInstance().camera.matrices.view;

	////Set other uniforms here too!
	uniformBufferData.camPos = m_pPlanet->GetTriangulator()->GetFrustum()->GetPositionOS();
	uniformBufferData.radius = m_pPlanet->GetRadius();
	uniformBufferData.morphRange = m_MorphRange;
	uniformBufferData.radius = m_pPlanet->GetRadius();
		
}

Patch::~Patch() {

	vertexBuffer.destroy();
	indexBuffer.destroy();
	instanceBuffer.destroy();
	uniformBuffer.destroy();

}