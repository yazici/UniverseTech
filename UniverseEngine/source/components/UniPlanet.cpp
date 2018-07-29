#include "UniPlanet.h"
#include "../UniEngine.h"
#include "../UniScene.h"
#include <assert.h>
#include <iostream>
#include "glm/gtx/quaternion.hpp"
#include "../FastNoise.h"
#include <math.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>



UniPlanet::UniPlanet(double radius /*= 1.0*/, double maxHeightOffset /*= 0.1*/, double maxDepthOffset /*= 0.1*/, uint16_t gridSize /*= 10*/, bool hasOcean /*= false*/){

	m_Radius = radius;
	m_MaxHeightOffset = maxHeightOffset;
	m_MaxDepthOffset = maxDepthOffset;
	m_GridSize = gridSize;
	m_HasOcean = hasOcean;

	std::cout << "Initializing planet..." << std::endl;
	std::cout << "Initializing planet complete." << std::endl;
}

void UniPlanet::Destroy() {

	std::cout << "Destroying planet..." << std::endl;

	auto& engine = UniEngine::GetInstance();
	engine.UnRegisterMaterial(m_Material);
	m_Material.reset();
	
}

void UniPlanet::Initialize() {

	auto& engine = UniEngine::GetInstance();
	
	m_Material = std::static_pointer_cast<PlanetMaterial>(MaterialFactory::create("planet", "testworld", m_HasOcean));

	engine.RegisterMaterial(m_Material);
	
	CreateGrid();
	CreateTriangles();
	UpdateMesh();
	CreateBuffers();
	UpdateStorageBuffer();
	UpdateBuffers();

	auto modelMat = glm::mat4(1.0);
	UpdateUniformBuffers(modelMat);

	MakeContintentTexture();
	MakeRampTexture();
	
	std::cout << "Created planet grid with " << m_GridPoints.size() << " points and " << m_Triangles.size() / 3 << " tris." << std::endl;
}

void UniPlanet::CreateGrid() {

	m_GridPoints.clear();
	float increment = 2.0f / m_GridSize;
	for(float y = -1.0f; y <= 1.0f; y+= increment) {
		for (float x = -1.0f; x <= 1.0f; x+= increment){
			float z = (1.0f - pow(x, 4.0f)) * (1.0f - pow(y, 4.0f));
			m_GridPoints.emplace_back(x, y, -z);
		}
	}


	assert(m_GridPoints.size() == (m_GridSize + 1) * (m_GridSize + 1));
}

void UniPlanet::CreateTriangles() {
	m_Triangles.clear();
	for(int y = 0; y < m_GridSize; y++) {
		for(int x = 0; x < m_GridSize; x++) {
			m_Triangles.push_back(y * (m_GridSize + 1) + x);
			m_Triangles.push_back((y + 1) * (m_GridSize + 1) + x);
			m_Triangles.push_back(y * (m_GridSize + 1) + x + 1);

			m_Triangles.push_back(y * (m_GridSize + 1) + x + 1);
			m_Triangles.push_back((y + 1) * (m_GridSize + 1) + x);
			m_Triangles.push_back((y + 1) * (m_GridSize + 1) + x + 1);

		}
	}

	assert(m_Triangles.size() == m_GridSize * m_GridSize * 6);
}

glm::vec3 UniPlanet::RotatePointToCamera(glm::vec3 point)
{
	glm::quat rot = glm::rotation(glm::vec3(0.f, 0.f, -1.f), m_CurrentCameraPos);
	return rot * point;
}

double UniPlanet::CalculateZOffset() {
	double d = glm::length(m_CurrentCameraPos);
	//std::cout << "Camera distance: " << d;
	double d2 = pow(d, 2.0);
	auto r = m_Radius;
	double r2 = pow(r, 2.0);
	
	double R = r + (r * m_MaxHeightOffset);
	double R2 = pow(R, 2.0);

	auto h = sqrt(d2 - r2);
	auto s = sqrt(R2 - r2);

	auto zs = R2 + d2 - pow(h + s, 2.0);
	zs /= ((2 * r * h) + (2 * r * s));

	return zs;

}

void UniPlanet::SetCameraPosition(glm::vec3& cam) {
	m_CurrentCameraPos = cam;
	//std::cout << "Camera relative to planet: " << cam.x << ", " << cam.y << ", " << cam.z << std::endl;
}

double UniPlanet::GetPositionOffset(glm::vec3& pos) {
	return glm::length(pos);
}

void UniPlanet::UpdateMesh() {
	m_MeshVerts.clear();
	m_OceanVerts.clear();
	auto zs = CalculateZOffset();
	//std::cout << ", planet Z offset: " << zs << std::endl;

	auto rot = glm::lookAt({ 0, 0, 0 }, m_CurrentCameraPos, { 0, 1, 0 });

	auto F = glm::normalize(m_CurrentCameraPos);
	auto Z = F + glm::vec3(1.f, 0.f, 0.f);
	auto U = glm::normalize(glm::cross(F, Z));
	auto R = glm::cross(F, U);

	//glm::mat3 rot(R, U, F);

	for(auto gp : m_GridPoints) {
		glm::vec3 point({ gp.x, gp.y, gp.z - zs });
		point = glm::vec4(point, 0) * rot;
		point = glm::normalize(point) * (float)m_Radius;
		//pos *= GetPositionOffset(gp);
		m_MeshVerts.push_back(point);

		if(m_HasOcean) {
			m_OceanVerts.push_back(point);
		}

	}

	
}


void UniPlanet::UpdateBuffers() {
	m_IndexCount = static_cast<uint32_t>(m_Triangles.size());
	m_VertexCount = static_cast<uint32_t>(m_MeshVerts.size());
	m_OceanVertexCount = static_cast<uint32_t>(m_OceanVerts.size());

	m_Material->SetIndexCount(m_IndexCount);

	uint32_t vertexBufferSize = static_cast<uint32_t>(m_MeshVerts.size() * sizeof(glm::vec3));
	uint32_t indexBufferSize = static_cast<uint32_t>(m_Triangles.size() * sizeof(uint32_t));

	vks::Buffer vertexStaging, indexStaging, oceanVertexStaging;

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

	if(m_HasOcean) {
		uint32_t oceanVertexBufferSize = static_cast<uint32_t>(m_OceanVerts.size() * sizeof(glm::vec3));
		// Vertex buffer
		VK_CHECK_RESULT(device->createBuffer(
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			&oceanVertexStaging,
			oceanVertexBufferSize,
			m_OceanVerts.data()));

		copyRegion.size = m_OceanVertexBuffer.size;
		vkCmdCopyBuffer(copyCmd, oceanVertexStaging.buffer, m_OceanVertexBuffer.buffer, 1, &copyRegion);
	}

	device->flushCommandBuffer(copyCmd, UniEngine::GetInstance().GetQueue());

	// Destroy staging resources
	vkDestroyBuffer(device->logicalDevice, vertexStaging.buffer, nullptr);
	vkFreeMemory(device->logicalDevice, vertexStaging.memory, nullptr);
	vkDestroyBuffer(device->logicalDevice, indexStaging.buffer, nullptr);
	vkFreeMemory(device->logicalDevice, indexStaging.memory, nullptr);
	if(m_HasOcean) {
		vkDestroyBuffer(device->logicalDevice, oceanVertexStaging.buffer, nullptr);
		vkFreeMemory(device->logicalDevice, oceanVertexStaging.memory, nullptr);
	}

}


void UniPlanet::UpdateUniformBuffers(glm::mat4& modelMat) {
	auto& engine = UniEngine::GetInstance();
	auto camera = engine.GetScene()->GetCameraComponent();

	//// Pass transformations to the shader
	m_UniformBufferData.modelMat = modelMat;
	m_UniformBufferData.viewMat = camera->matrices.view;
	m_UniformBufferData.projMat = camera->matrices.projection;

	////Set other uniforms here too!
	auto camPos = glm::vec3(glm::inverse(modelMat) * glm::vec4(camera->GetPosition(), 1.f));
	m_UniformBufferData.camPos = glm::vec4(camPos, 1.0);
	m_UniformBufferData.radius = (float)m_Radius;
	m_UniformBufferData.maxHeight = (float)m_MaxHeightOffset;
	m_UniformBufferData.minDepth = (float)m_MaxDepthOffset;

	m_UniformBufferData.tessAlpha = 1.0f;
	
	auto camDist = glm::length(camPos) - (float)m_Radius;
	auto doublings = log(camDist);
	doublings /= 2.0f;
	doublings = glm::clamp(doublings, 0.f, 8.f);
	doublings = round(doublings);
	m_UniformBufferData.tessLevel = 8.0f - doublings;


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

// TODO: Fixme for pn-patch interpolation causing offset problems where shader != cpp
float UniPlanet::GetAltitude(glm::vec3& point) {

	//std::cout << "Input pos: " << point.x << ", " << point.y << ", " << point.z;

	auto p = glm::normalize(point);

	float u = glm::atan(p.z, p.x) / (2 * 3.1415926f) + 0.5f;
	float v = p.y * 0.5f + 0.5f;

	uint32_t x = (uint32_t)round(u * 1024.f);
	uint32_t y = (uint32_t)round(v * 1024.f);

	x = x % 1024;
	y = y % 1024;

	auto n = m_ContinentData[y * 1024 + x];

	//std::cout << ". Lookup offset data: " << x << ", " << y << " = " << n;

	n = std::clamp(n, 0.5f, 1.f);

	auto height = (float)m_Radius + float(m_Radius) * (float)m_MaxHeightOffset * n;

	//std::cout << ", height: " << height << std::endl;

	auto altitude = glm::length(point) - (float)height;

	return altitude;

}

void UniPlanet::SetZOffset(float  value) {
	m_ZOffset = value;
}

void UniPlanet::CreateBuffers() {

	uint32_t vertexBufferSize = static_cast<uint32_t>(m_MeshVerts.size() * sizeof(glm::vec3));
	uint32_t oceanVertexBufferSize = static_cast<uint32_t>(m_OceanVerts.size() * sizeof(glm::vec3));
	uint32_t indexBufferSize = static_cast<uint32_t>(m_Triangles.size() * sizeof(uint32_t));
	uint32_t ubSize = static_cast<uint32_t>(sizeof(UniPlanet::UniformBufferData));
	uint32_t storageBufferSize = static_cast<uint32_t>(m_NoiseLayers.size() * sizeof(NoiseLayerData));

	auto device = UniEngine::GetInstance().vulkanDevice;

	// Create device local target buffers
	// Vertex buffer
	VK_CHECK_RESULT(device->createBuffer(
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		&m_VertexBuffer,
		vertexBufferSize));


	if(m_HasOcean) {

		// Create device local target buffers
		// Vertex buffer
		VK_CHECK_RESULT(device->createBuffer(
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			&m_OceanVertexBuffer,
			oceanVertexBufferSize));
	}

	// Index buffer
	VK_CHECK_RESULT(device->createBuffer(
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		&m_IndexBuffer,
		indexBufferSize));

	// uniform buffer
	VK_CHECK_RESULT(device->createBuffer(
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		&m_UniformBuffer, ubSize));

	// storage buffer
	VK_CHECK_RESULT(device->createBuffer(
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		&m_StorageBuffer, storageBufferSize));


	m_Material->SetBuffer("uniform", std::make_shared<vks::Buffer>(m_UniformBuffer));
	m_Material->SetBuffer("noiselayers", std::make_shared<vks::Buffer>(m_StorageBuffer));
	m_Material->SetBuffer("vertex", std::make_shared<vks::Buffer>(m_VertexBuffer));
	m_Material->SetBuffer("index", std::make_shared<vks::Buffer>(m_IndexBuffer));
	if(m_HasOcean) {
		m_Material->SetBuffer("oceanvertex", std::make_shared<vks::Buffer>(m_OceanVertexBuffer));
	}
}

void UniPlanet::DestroyBuffers() {
	m_IndexBuffer.destroy();
	m_VertexBuffer.destroy();
	if(m_HasOcean) {
		m_OceanVertexBuffer.destroy();
	}
	m_UniformBuffer.destroy();
}


// TODO: height calculation needs to happen in tess or geometry shader
void UniPlanet::MakeContintentTexture() {

	FastNoise noise;
	noise.SetNoiseType(FastNoise::SimplexFractal);
	noise.SetFractalOctaves(3);

	auto& engine = UniEngine::GetInstance();

	std::vector<glm::vec4> buffer;

	glm::vec3 noiseOffset = { 2, 2, 2 };
	float noiseScale = 120.3587f;

	for(float lat = -90.f; lat < 90.f; lat += 180.f / 1024) {
		for(float lon = 0; lon < 360.f; lon += 360.f / 1024) {
			auto x = cos(glm::radians(lat)) * cos(glm::radians(lon));
			auto y = cos(glm::radians(lat)) * sin(glm::radians(lon));
			auto z = sin(glm::radians(lat));

			auto nv = (glm::normalize(glm::vec3(x, y, z)) + noiseOffset) * noiseScale;

			float n = noise.GetNoise(nv.x, nv.y, nv.z) / 2.f + 0.5f;
			m_ContinentData.push_back(n);
			buffer.emplace_back(n, n, n, 0);
		}
	}

	m_ContinentTexture.fromBuffer(buffer.data(), buffer.size() * sizeof(glm::vec4), VK_FORMAT_R32G32B32A32_SFLOAT, 1024, 1024, engine.vulkanDevice, engine.GetQueue(), VK_FILTER_LINEAR);

	auto t = make_shared<vks::Texture>(m_ContinentTexture);

	m_Material->m_Textures.push_back(t);
}

void UniPlanet::UpdateStorageBuffer() {
	auto& engine = UniEngine::GetInstance();
	
	vks::Buffer storageStaging;
	auto device = engine.vulkanDevice;

	m_Material->SetNoiseLayerCount(static_cast<uint32_t>(m_NoiseLayers.size()));

	// Vertex buffer
	VK_CHECK_RESULT(device->createBuffer(
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&storageStaging,
		static_cast<uint32_t>(m_NoiseLayers.size() * sizeof(NoiseLayerData)),
		m_NoiseLayers.data()));

	// Copy from staging buffers
	VkCommandBuffer copyCmd = device->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

	VkBufferCopy copyRegion{};

	copyRegion.size = static_cast<uint32_t>(m_NoiseLayers.size() * sizeof(NoiseLayerData));
	vkCmdCopyBuffer(copyCmd, storageStaging.buffer, m_StorageBuffer.buffer, 1, &copyRegion);

	device->flushCommandBuffer(copyCmd, engine.GetQueue());

	// Destroy staging resources
	vkDestroyBuffer(device->logicalDevice, storageStaging.buffer, nullptr);
	vkFreeMemory(device->logicalDevice, storageStaging.memory, nullptr);
}

double UniPlanet::GetRadius() {
	return m_Radius;
}

uint32_t UniPlanet::AddNoiseLayer(NoiseType type, uint32_t octaves, float seed/*=1337.0f*/) {
	auto nl = NoiseLayerData();
	nl.type = static_cast<uint32_t>(type);
	nl.octaves = octaves;
	nl.seed = seed;
	m_NoiseLayers.push_back(nl);
	return static_cast<uint32_t>(m_NoiseLayers.size() - 1);
}

void UniPlanet::MakeRampTexture() {
	auto& engine = UniEngine::GetInstance();
	std::string path = getAssetPath() + "textures/terrain-ramp.png";
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

	VkDeviceSize imSize = texWidth * texHeight * 4;
	
	m_RampTexture.fromBuffer(pixels, imSize, VK_FORMAT_R8G8B8A8_UNORM, texWidth, texHeight, engine.vulkanDevice, engine.GetQueue(), VK_FILTER_LINEAR);

	auto t = make_shared<vks::Texture>(m_RampTexture);

	m_Material->m_Textures.push_back(t);
}
