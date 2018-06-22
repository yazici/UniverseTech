#include "Patch.hpp"

#include "Components.h"
// #include "../Components/CameraComponent.hpp"
#include "UniEngine.h"
#include "UniBody.h"
#include "UniFrustum.hpp"
#include "Triangulator.hpp"
#include "vks/VulkanTools.h"


vks::Buffer* Patch::GetInstanceBuffer() {
	return &instanceBuffers[instanceBuffers.size() - 1];
}

void Patch::MakeInstanceBuffer(uint32_t size) {

	//if(instanceBuffers.size() > 4) {
	//	instanceBuffers[0].destroy();
	//	std::vector<vks::Buffer> tmp;
	//	for(int i = 1; i < instanceBuffers.size(); i++) {
	//		tmp.emplace_back(instanceBuffers[i]);
	//	}
	//	instanceBuffers = tmp;
	//}

	auto device = UniEngine::GetInstance().vulkanDevice;
	vks::Buffer instanceBuffer;

	VK_CHECK_RESULT(device->createBuffer(
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		&instanceBuffer,
		size));

	instanceBuffers.push_back(instanceBuffer);
}

Patch::Patch(uint16_t levels)
	:m_Levels(levels) {}

void Patch::Init() {

	auto device = UniEngine::GetInstance().vulkanDevice;

	uint32_t ubSize = static_cast<uint32_t>(sizeof(UniformBufferData));

	VK_CHECK_RESULT(device->createBuffer(
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		&uniformBuffer, ubSize));


	MakeInstanceBuffer(256 * sizeof(PatchInstance));

	vertexDescription.bindingDescriptions.resize(2);
	vertexDescription.bindingDescriptions = {
		// Binding point 0: Mesh vertex layout description at per-vertex rate
		vks::initializers::vertexInputBindingDescription(VERTEX_BUFFER_BIND_ID, sizeof(glm::vec2) * 2, VK_VERTEX_INPUT_RATE_VERTEX),
		// Binding point 1: Instanced data at per-instance rate
		vks::initializers::vertexInputBindingDescription(INSTANCE_BUFFER_BIND_ID, sizeof(PatchInstance), VK_VERTEX_INPUT_RATE_INSTANCE)
	};


	// PER VERTEX

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
	
	
	// PER INSTANCE
	
	offset = 0;
	// Location 2: Level
	vertexDescription.attributeDescriptions[2] =
		vks::initializers::vertexInputAttributeDescription(
			INSTANCE_BUFFER_BIND_ID,
			2,
			VK_FORMAT_R32_UINT,
			offset);
	offset += sizeof(uint32_t);
	// Location 3: A
	vertexDescription.attributeDescriptions[3] =
		vks::initializers::vertexInputAttributeDescription(
			INSTANCE_BUFFER_BIND_ID,
			3,
			VK_FORMAT_R32G32B32_SFLOAT,
			offset);
	offset += sizeof(glm::vec3);
	// Location 4: R
	vertexDescription.attributeDescriptions[4] =
		vks::initializers::vertexInputAttributeDescription(
			INSTANCE_BUFFER_BIND_ID,
			4,
			VK_FORMAT_R32G32B32_SFLOAT,
			offset);
	offset += sizeof(glm::vec3);
	// Location 5: S
	vertexDescription.attributeDescriptions[5] =
		vks::initializers::vertexInputAttributeDescription(
			INSTANCE_BUFFER_BIND_ID,
			5,
			VK_FORMAT_R32G32B32_SFLOAT,
			offset);

	vertexDescription.inputState = vks::initializers::pipelineVertexInputStateCreateInfo();
	vertexDescription.inputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexDescription.attributeDescriptions.size());
	vertexDescription.inputState.pVertexAttributeDescriptions = vertexDescription.attributeDescriptions.data();
	vertexDescription.inputState.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexDescription.bindingDescriptions.size());
	vertexDescription.inputState.pVertexBindingDescriptions = vertexDescription.bindingDescriptions.data();
	

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

	indexCount = static_cast<uint32_t>(m_Indices.size());
	vertexCount = static_cast<uint32_t>(m_Vertices.size());

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

	if(m_NumInstances == 0) {
		return;
	}
	
	auto device = UniEngine::GetInstance().vulkanDevice;
	auto neededSize = m_NumInstances * sizeof(PatchInstance);
	/*auto requestedSize = neededSize * 2;

	vks::Buffer* instanceBuffer = GetInstanceBuffer();

	if(neededSize > instanceBuffer->size) {
		MakeInstanceBuffer(requestedSize);
		instanceBuffer = GetInstanceBuffer();
	}*/

	m_instanceBuffer.destroy();

	vks::Buffer instanceStaging;

	// Vertex buffer
	VK_CHECK_RESULT(device->createBuffer(
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&instanceStaging,
		neededSize,
		instances.data()));

	// Create device local target buffers
	// Vertex buffer
	VK_CHECK_RESULT(device->createBuffer(
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		&m_instanceBuffer,
		neededSize));

	// Copy from staging buffers
	VkCommandBuffer copyCmd = device->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

	VkBufferCopy copyRegion{};

	copyRegion.size = neededSize;
	vkCmdCopyBuffer(copyCmd, instanceStaging.buffer, m_instanceBuffer.buffer, 1, &copyRegion);

	device->flushCommandBuffer(copyCmd, UniEngine::GetInstance().GetQueue());

	// Destroy staging resources
	vkDestroyBuffer(device->logicalDevice, instanceStaging.buffer, nullptr);
	vkFreeMemory(device->logicalDevice, instanceStaging.memory, nullptr);

}

void Patch::UploadDistanceLUT(std::vector<float> &distances) {
	for(size_t i = 0; i < distances.size(); i++) {
		uniformBufferData.distanceLut[i*4] = distances[i]; // dumb uniform float array packing!
	}
}

void Patch::Draw() {
	
	//// Pass transformations to the shader
	uniformBufferData.model = m_pPlanet->GetTransform()->GetModelMat();
	uniformBufferData.viewProj = UniEngine::GetInstance().camera.matrices.perspective * UniEngine::GetInstance().camera.matrices.view;

	////Set other uniforms here too!
	uniformBufferData.camPos = m_pPlanet->GetTriangulator()->GetFrustum()->GetPositionOS();
	uniformBufferData.radius = (float)m_pPlanet->GetRadius();
	uniformBufferData.morphRange = m_MorphRange;

	vks::Buffer uniformStaging;
	auto device = UniEngine::GetInstance().vulkanDevice;

	// Vertex buffer
	VK_CHECK_RESULT(device->createBuffer(
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&uniformStaging,
		static_cast<uint32_t>(sizeof(UniformBufferData)),
		&uniformBufferData));

	// Copy from staging buffers
	VkCommandBuffer copyCmd = device->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

	VkBufferCopy copyRegion{};

	copyRegion.size = static_cast<uint32_t>(sizeof(UniformBufferData));
	vkCmdCopyBuffer(copyCmd, uniformStaging.buffer, uniformBuffer.buffer, 1, &copyRegion);

	device->flushCommandBuffer(copyCmd, UniEngine::GetInstance().GetQueue());

	// Destroy staging resources
	vkDestroyBuffer(device->logicalDevice, uniformStaging.buffer, nullptr);
	vkFreeMemory(device->logicalDevice, uniformStaging.memory, nullptr);

}

Patch::~Patch() {

	vertexBuffer.destroy();
	indexBuffer.destroy();
	for(auto & instanceBuffer : instanceBuffers)
		instanceBuffer.destroy();

	uniformBuffer.destroy();

}