#pragma once
#include <glm/glm.hpp>
#include "../vks/VulkanBuffer.hpp"
#include "../vks/VulkanTexture.hpp"
#include "../UniMaterial.h"

class UniPlanet{
public:

	struct UniformBufferData {
		glm::mat4 modelMat;
		glm::mat4 viewMat;
		glm::mat4 projMat;
		glm::vec4 camPos;
		float radius;
		float maxHeight;
		float minDepth;
		float tessLevel;
		float tessAlpha;
		bool hasOcean = false;
	} m_UniformBufferData;

	vks::Buffer m_VertexBuffer;
	vks::Buffer m_OceanVertexBuffer;
	vks::Buffer m_IndexBuffer;
	vks::Buffer m_UniformBuffer;
	uint32_t m_VertexCount;
	uint32_t m_IndexCount;
	VkDescriptorSet m_DescriptorSet;


	UniPlanet(double radius = 1.0, double maxHeightOffset = 0.1, double maxDepthOffset = 0.1, uint16_t gridSize = 10, bool hasOcean = false);

	void Destroy();
	virtual ~UniPlanet() { Destroy(); }

	void Initialize();
	void CreateGrid();
	void CreateTriangles();
	glm::vec3 RotatePointToCamera(glm::vec3 point);
	double CalculateZOffset();
	void SetCameraPosition(glm::vec3& cam);
	double GetPositionOffset(glm::vec3& pos);
	void UpdateMesh();
	void UpdateBuffers();
	void UpdateUniformBuffers(glm::mat4& modelMat);
	float GetAltitude(glm::vec3& point);
	glm::vec3 CameraPos() { return m_CurrentCameraPos; }
	std::vector<glm::vec3> GetMesh() { return m_MeshVerts; }
	uint16_t GridSize() { return m_GridSize; }

	std::shared_ptr<PlanetMaterial> m_Material;

	void SetZOffset(float value);

	double GetRadius();

private:

	double m_Radius = 1.0;
	double m_MaxHeightOffset = 0.001;
	double m_MaxDepthOffset = 0.001;

	uint16_t m_GridSize = 10;
	double m_GridZOffset = 0.0;

	std::vector<glm::vec3> m_GridPoints;
	std::vector<glm::vec3> m_MeshVerts;
	std::vector<glm::vec3> m_OceanVerts;
	std::vector<uint32_t> m_Triangles;

	glm::vec3 m_CurrentCameraPos = glm::vec3(0, 0, 10);

	void CreateBuffers();
	void DestroyBuffers();
	float m_ZOffset;
	

	vks::Texture2D m_ContinentTexture;
	vks::Texture2D m_RampTexture;
	void MakeRampTexture();
	void MakeContintentTexture();

	std::vector<float> m_ContinentData;
	bool m_HasOcean = false;
	uint32_t m_OceanVertexCount;
};

