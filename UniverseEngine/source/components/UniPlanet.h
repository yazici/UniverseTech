#pragma once
#include <glm/glm.hpp>
#include "../vks/VulkanBuffer.hpp"
#include "../UniMaterial.h"

class UniPlanet{
public:

	struct UniformBufferData {
		glm::mat4 modelMat;
		glm::mat4 viewMat;
		glm::mat4 projMat;
		glm::vec4 camPos;
		double radius;
		double maxHeight;
		double minDepth;
	} m_UniformBufferData;

	vks::Buffer m_VertexBuffer;
	vks::Buffer m_IndexBuffer;
	vks::Buffer m_UniformBuffer;
	uint32_t m_VertexCount;
	uint32_t m_IndexCount;
	VkDescriptorSet m_DescriptorSet;


	UniPlanet(double radius = 1.0, double maxHeightOffset = 0.1, double maxDepthOffset = 0.1, uint16_t gridSize = 10);

	void Destroy();
	virtual ~UniPlanet() = default;

	void Initialize();
	void CreateGrid();
	void CreateTriangles();
	std::vector<glm::vec3> RotateGridToCamera();
	double CalculateZOffset();
	void SetCameraPosition(glm::vec3& cam);
	double GetPositionOffset(glm::vec3& pos);
	void UpdateMesh();
	void UpdateBuffers();
	void UpdateUniformBuffers(glm::mat4& modelMat);

	std::shared_ptr<PlanetMaterial> m_Material;

private:

	double m_Radius = 1.0;
	double m_MaxHeightOffset = 0.1;
	double m_MaxDepthOffset = 0.1;

	uint16_t m_GridSize = 10;
	double m_GridZOffset = 0.0;

	std::vector<glm::vec3> m_GridPoints;
	std::vector<glm::vec3> m_MeshVerts;
	std::vector<uint32_t> m_Triangles;

	glm::vec3 m_CurrentCameraPos = glm::vec3(0, 0, 10);

	void CreateBuffers();
	void DestroyBuffers();
};

