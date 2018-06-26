#pragma once

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <vector>

#include "vulkan/vulkan.h"
#include "vks/VulkanDevice.hpp"
#include "vks/VulkanBuffer.hpp"

class UniBody;


struct PatchVertex {
	PatchVertex(glm::vec2 position, glm::vec2 morphVec) {
		pos = position;
		morph = morphVec;
	}
	glm::vec2 pos;
	glm::vec2 morph;
};

struct PatchInstance {
	PatchInstance(uint16_t Level, glm::vec3 A, glm::vec3 R, glm::vec3 S) {
		level = Level;
		a = A;
		r = R;
		s = S;
	}
	uint32_t level;
	glm::vec3 a;
	glm::vec3 r;
	glm::vec3 s;
};

class Patch {
public:

	struct UniformBufferData {

		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
		float distanceLut[32*4]; // stupid uniform float packing!
		glm::vec3 camPos;
		float radius;
		float morphRange;
		float maxHeight;

	} uniformBufferData;

	vks::Buffer uniformBuffer;
	vks::Buffer m_instanceBuffer;
	vks::Buffer* GetInstanceBuffer();
	void MakeInstanceBuffer(uint32_t size);
	std::vector<vks::Buffer> instanceBuffers;

	struct {
		VkPipelineVertexInputStateCreateInfo inputState;
		std::vector<VkVertexInputBindingDescription> bindingDescriptions;
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
	} vertexDescription;


	Patch(uint16_t levels = 5);
	~Patch();

	void SetPlanet(UniBody* pPlanet) { m_pPlanet = pPlanet; }

	uint32_t GetVertexCount() { return static_cast<uint32_t>(m_Vertices.size()); }

	void Init();
	void GenerateGeometry(uint16_t levels);
	void BindInstances(std::vector<PatchInstance> &instances);
	void UploadDistanceLUT(std::vector<float> &distances);
	void Draw();

	vks::Buffer vertexBuffer;
	vks::Buffer indexBuffer;
	uint32_t indexCount = 0;
	uint32_t vertexCount = 0;
	uint32_t m_NumInstances = 0;

	VkDescriptorSet m_DescriptorSet;

private:
	std::vector<PatchVertex>m_Vertices;
	std::vector<uint32_t>m_Indices;

	UniBody *m_pPlanet = nullptr;

	uint16_t m_Levels;
	uint32_t m_RC;

	float m_MorphRange = 0.5f;

	glm::vec3 m_Ambient = glm::vec3(0.05f, 0.05f, 0.08f);
	
};