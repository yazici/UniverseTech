#include "UniPlanet.h"
#include "../UniEngine.h"
#include <assert.h>
#include <iostream>


UniPlanet::~UniPlanet() {
	DestroyBuffers();
}

void UniPlanet::Initialize() {

	auto device = UniEngine::GetInstance().vulkanDevice;
	uint32_t ubSize = static_cast<uint32_t>(sizeof(UniPlanet::UniformBufferData));

	VK_CHECK_RESULT(device->createBuffer(
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		&m_UniformBuffer, ubSize));

	m_VertexDescription.bindingDescriptions.resize(1);
	m_VertexDescription.bindingDescriptions = {
		// Binding point 0: Mesh vertex layout description at per-vertex rate
		vks::initializers::vertexInputBindingDescription(VERTEX_BUFFER_BIND_ID, sizeof(glm::vec3), VK_VERTEX_INPUT_RATE_VERTEX),
	};


	// PER VERTEX

	// Attribute descriptions
	m_VertexDescription.attributeDescriptions.resize(1);
	uint32_t offset = 0;
	// Location 0: Pos
	m_VertexDescription.attributeDescriptions[0] =
		vks::initializers::vertexInputAttributeDescription(
			VERTEX_BUFFER_BIND_ID,
			0,
			VK_FORMAT_R32G32B32_SFLOAT,
			offset);

	m_VertexDescription.inputState = vks::initializers::pipelineVertexInputStateCreateInfo();
	m_VertexDescription.inputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(m_VertexDescription.attributeDescriptions.size());
	m_VertexDescription.inputState.pVertexAttributeDescriptions = m_VertexDescription.attributeDescriptions.data();
	m_VertexDescription.inputState.vertexBindingDescriptionCount = static_cast<uint32_t>(m_VertexDescription.bindingDescriptions.size());
	m_VertexDescription.inputState.pVertexBindingDescriptions = m_VertexDescription.bindingDescriptions.data();


	CreateGrid();
	CreateTriangles();
	UpdateMesh();
	CreateBuffers();
	UpdateBuffers();

	auto modelMat = glm::mat4();
	UpdateUniformBuffers(modelMat);
	
	std::cout << "Created planet grid with " << m_GridPoints.size() << " points and " << m_Triangles.size() << " tris." << std::endl;
}

void UniPlanet::CreateGrid() {

	m_GridPoints.clear();
	double increment = 2.0 / m_GridSize;
	for(double y = -1.0; y <= 1.0; y+= increment) {
		for (double x = -1.0; x <= 1.0; x+= increment){
			double z = (1.0 - pow(x, 4.0)) * (1.0 - pow(y, 4.0));
			m_GridPoints.emplace_back(x, y, z);
		}
	}

	assert(m_GridPoints.size() == (m_GridSize + 1) * (m_GridSize + 1));
}

void UniPlanet::CreateTriangles() {
	m_Triangles.clear();
	for(int y = 0; y <= m_GridSize; y++) {
		for(int x = 0; x <= m_GridSize; x++) {
			m_Triangles.push_back(y * (m_GridSize + 1) + x);
			m_Triangles.push_back(y * (m_GridSize + 1) + x + 1);
			m_Triangles.push_back((y + 1) * (m_GridSize + 1) + x);

			m_Triangles.push_back(y * (m_GridSize + 1) + x + 1);
			m_Triangles.push_back((y + 1) * (m_GridSize + 1) + x + 1);
			m_Triangles.push_back((y + 1) * (m_GridSize + 1) + x);
		}
	}

	assert(m_Triangles.size() == (m_GridSize + 1) * (m_GridSize + 1) * 6);
}

std::vector<glm::vec3> UniPlanet::RotateGridToCamera()
{

	glm::quat rot = glm::rotation(glm::vec3(0, 0, 1), m_CurrentCameraPos);

	std::vector<glm::vec3> rotatedPoints;

	for(auto gp : m_GridPoints) {
		rotatedPoints.push_back(gp);
	}

	return rotatedPoints;
}

double UniPlanet::CalculateZOffset() {
	double d = glm::length(m_CurrentCameraPos);
	double d2 = pow(d, 2.0);
	auto r = m_Radius;
	double r2 = pow(r, 2.0);
	
	double R = r + m_MaxHeightOffset;
	double R2 = pow(R, 2.0);

	auto h = sqrt(d2 - r2);
	auto s = sqrt(R2 - r2);

	auto z = (R2 + d2 - pow(h + s, 2.0)) / ((2 * r) * (h + s));

	return z;

}

void UniPlanet::SetCameraPosition(glm::vec3& cam) {
	m_CurrentCameraPos = cam;
}

double UniPlanet::GetPositionOffset(glm::vec3& pos) {
	return glm::length(pos);
}

void UniPlanet::UpdateMesh() {
	m_MeshVerts.clear();
	auto zs = CalculateZOffset();
	auto gridPoints = RotateGridToCamera();
	for(auto gp : gridPoints) {
		gp.z += (float)zs;
		auto pos = glm::normalize(gp);
		pos *= GetPositionOffset(gp);
		m_MeshVerts.push_back(pos);
	}
}


void UniPlanet::UpdateBuffers() {
	m_IndexCount = static_cast<uint32_t>(m_Triangles.size());
	m_VertexCount = static_cast<uint32_t>(m_MeshVerts.size());

	uint32_t vertexBufferSize = static_cast<uint32_t>(m_MeshVerts.size() * sizeof(glm::vec3));
	uint32_t indexBufferSize = static_cast<uint32_t>(m_Triangles.size() * sizeof(uint32_t));

	vks::Buffer vertexStaging, indexStaging;

	auto device = UniEngine::GetInstance().vulkanDevice;

	// Vertex buffer
	VK_CHECK_RESULT(device->createBuffer(
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&vertexStaging,
		vertexBufferSize,
		m_MeshVerts.data()));

	// Index buffer
	VK_CHECK_RESULT(device->createBuffer(
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&indexStaging,
		indexBufferSize,
		m_Triangles.data()));

	// Copy from staging buffers
	VkCommandBuffer copyCmd = device->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

	VkBufferCopy copyRegion{};

	copyRegion.size = m_VertexBuffer.size;
	vkCmdCopyBuffer(copyCmd, vertexStaging.buffer, m_VertexBuffer.buffer, 1, &copyRegion);

	copyRegion.size = m_IndexBuffer.size;
	vkCmdCopyBuffer(copyCmd, indexStaging.buffer, m_IndexBuffer.buffer, 1, &copyRegion);

	device->flushCommandBuffer(copyCmd, UniEngine::GetInstance().GetQueue());

	// Destroy staging resources
	vkDestroyBuffer(device->logicalDevice, vertexStaging.buffer, nullptr);
	vkFreeMemory(device->logicalDevice, vertexStaging.memory, nullptr);
	vkDestroyBuffer(device->logicalDevice, indexStaging.buffer, nullptr);
	vkFreeMemory(device->logicalDevice, indexStaging.memory, nullptr);

}


void UniPlanet::UpdateUniformBuffers(glm::mat4& modelMat) {
	auto camera = UniEngine::GetInstance().GetScene()->GetCameraComponent();

	//// Pass transformations to the shader
	m_UniformBufferData.modelMat = modelMat;
	m_UniformBufferData.viewMat = camera->matrices.view;
	m_UniformBufferData.projMat = camera->matrices.projection;

	////Set other uniforms here too!
	auto camPos = glm::vec3(glm::inverse(modelMat) * glm::vec4(camera->GetPosition(), 1.f));
	m_UniformBufferData.camPos = glm::vec4(camPos, 1.0);
	m_UniformBufferData.radius = m_Radius;
	m_UniformBufferData.maxHeight = m_MaxHeightOffset;
	m_UniformBufferData.minDepth = m_MaxDepthOffset;

	vks::Buffer uniformStaging;
	auto device = UniEngine::GetInstance().vulkanDevice;

	// Vertex buffer
	VK_CHECK_RESULT(device->createBuffer(
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&uniformStaging,
		static_cast<uint32_t>(sizeof(UniformBufferData)),
		&m_UniformBufferData));

	// Copy from staging buffers
	VkCommandBuffer copyCmd = device->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

	VkBufferCopy copyRegion{};

	copyRegion.size = static_cast<uint32_t>(sizeof(UniformBufferData));
	vkCmdCopyBuffer(copyCmd, uniformStaging.buffer, m_UniformBuffer.buffer, 1, &copyRegion);

	device->flushCommandBuffer(copyCmd, UniEngine::GetInstance().GetQueue());

	// Destroy staging resources
	vkDestroyBuffer(device->logicalDevice, uniformStaging.buffer, nullptr);
	vkFreeMemory(device->logicalDevice, uniformStaging.memory, nullptr);

}

void UniPlanet::CreateBuffers() {

	uint32_t vertexBufferSize = static_cast<uint32_t>(m_MeshVerts.size() * sizeof(glm::vec3));
	uint32_t indexBufferSize = static_cast<uint32_t>(m_Triangles.size() * sizeof(uint32_t));

	auto device = UniEngine::GetInstance().vulkanDevice;

	// Create device local target buffers
	// Vertex buffer
	VK_CHECK_RESULT(device->createBuffer(
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		&m_VertexBuffer,
		vertexBufferSize));

	// Index buffer
	VK_CHECK_RESULT(device->createBuffer(
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		&m_IndexBuffer,
		indexBufferSize));


}

void UniPlanet::DestroyBuffers() {
	m_IndexBuffer.destroy();
	m_VertexBuffer.destroy();
	m_UniformBuffer.destroy();
}
