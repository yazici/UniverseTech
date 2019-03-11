#pragma once

#include "../vks/VulkanBuffer.hpp"
#include "../vks/VulkanTexture.hpp"
#include "../materials/PlanetMaterial.h"
#include "../vks/frustum.hpp"
#include "../3dmaths.h"

class Planet{
public:
	enum NoiseType {
		SIMPLEX,
		WORLEY_P1,
		WORLEY_P2,
		WORLEY_P1_MINUS_P2
	};

	struct UniformBufferData {
		glm::mat4 modelMat;
		glm::mat4 viewMat;
		glm::mat4 projMat;
		glm::vec4 camPos;
		float radius;
		float maxHeight;
		float minDepth;
		float tessLevel;
		glm::vec4 frustumPlanes[6];
		glm::vec2 viewportDim;
		float tessellatedEdgeSize;
		bool hasOcean = false;
	} m_UniformBufferData;

	struct NoiseLayerData {
		uint32_t type;
		uint32_t octaves = 1;
		float seed = 1337.0f;
		float scale = 1.0f;
		float amplitude = 0.5f;
		float gain = 0.5f;
		float lacunarity = 2.0f;
		float range_min = 0.0f;
		float range_max = 1.0f;
	};

	vks::Buffer m_VertexBuffer;
	vks::Buffer m_OceanVertexBuffer;
	vks::Buffer m_IndexBuffer;
	vks::Buffer m_OceanIndexBuffer;
	vks::Buffer m_UniformBuffer;
	vks::Buffer m_StorageBuffer;
	uint32_t m_VertexCount;
	uint32_t m_IndexCount;
	VkDescriptorSet m_DescriptorSet;

	vks::Frustum frustum;


	Planet(double radius = 1.0, double maxHeightOffset = 0.1, double maxDepthOffset = 0.1, uint16_t gridSize = 10, bool hasOcean = false);

	void Destroy();
	virtual ~Planet() { Destroy(); }

	void Initialize();
	void CreateGrid();
	void CreateOceanTriangles();
	void CreateQuads();
	glm::vec3 RotatePointToCamera(glm::vec3 point);
	double CalculateZOffset();
	double CalculateOceanZOffset();
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
	uint32_t AddNoiseLayer(NoiseType type, uint32_t octaves, float seed=1337.0f);
	void SetNoiseParam(uint32_t index, std::string param, float value);
	void UpdateTime(float t);

private:

	double m_Radius = 1.0;
	double m_MaxHeightOffset = 0.001;
	double m_MaxDepthOffset = 0.001;

	uint16_t m_GridSize = 10;
	double m_GridZOffset = 0.0;

	double m_MaxWaveHeight = 20.0;

	std::vector<glm::vec3> m_GridPoints;
	std::vector<glm::vec3> m_MeshVerts;
	std::vector<glm::vec3> m_OceanVerts;
	std::vector<uint32_t> m_Indices;
	std::vector<uint32_t> m_OceanIndices;

	std::vector<NoiseLayerData> m_NoiseLayers;

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
	void UpdateStorageBuffer();
	
	uint32_t m_OceanIndexCount;

};

