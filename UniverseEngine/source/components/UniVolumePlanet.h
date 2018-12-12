#pragma once
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include "../vks/VulkanBuffer.hpp"
#include "../vks/VulkanTexture.hpp"
#include "../UniMaterial.h"
#include <unordered_set>
#include <vector>
#include <list>


template <class T>
inline void hash_combine(std::size_t& seed, const T& v) {
	std::hash<T> hasher;
	seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

struct Diamond {
	glm::dvec3 pos;
	uint16_t type;
	uint16_t level;
	uint16_t j;
};

struct Tet {
	glm::dvec3 v0;
	glm::dvec3 v1;
	glm::dvec3 v2;
	glm::dvec3 v3;

	size_t h;

	Tet(glm::dvec3 a, glm::dvec3 b, glm::dvec3 c, glm::dvec3 d) {
		v0 = a;
		v1 = b;
		v2 = c;
		v3 = d;

		// Compute individual hash values for two data members and combine them using XOR and bit shifting
		auto h0 = hash<glm::dvec3>()(v0);
		auto h1 = hash<glm::dvec3>()(v1);
		auto h2 = hash<glm::dvec3>()(v2);
		auto h3 = hash<glm::dvec3>()(v3);

		std::list<size_t> hashes = { h0, h1, h2, h3 };
		hashes.sort();

		size_t seed = 0;
		for(auto& hx : hashes) {
			hash_combine(seed, hx);
		}

		h = seed;

	}

	std::shared_ptr<Diamond> d;

	uint16_t type = 0;
	uint16_t level = 0;
	uint16_t j = 19;

	std::pair<glm::dvec3, glm::dvec3> le;

	void CalculateLongestEdge() {
		std::pair<glm::dvec3, glm::dvec3> v01 = { v0, v1 };
		std::pair<glm::dvec3, glm::dvec3> v02 = { v0, v2 };
		std::pair<glm::dvec3, glm::dvec3> v03 = { v0, v3 };
		std::pair<glm::dvec3, glm::dvec3> v12 = { v1, v2 };
		std::pair<glm::dvec3, glm::dvec3> v13 = { v1, v3 };
		std::pair<glm::dvec3, glm::dvec3> v23 = { v2, v3 };
		std::vector<std::pair<glm::dvec3, glm::dvec3>> pairs = { v01, v02, v03, v12, v13, v23 };
		glm::dvec3 c;
		std::pair<glm::dvec3, glm::dvec3> cvs;
		double longest = 0;
		for(auto& p : pairs) {
			auto distance = glm::distance(p.first, p.second);
			if(distance > longest) {
				longest = distance;
				c = (p.first + p.second) / 2.0;
				cvs = p;
			}
		}
		d = std::make_shared<Diamond>();
		d->pos = c;
		d->level = level;
		d->j = j;
		le = cvs;
	}

	bool operator==(const Tet& rhs) const{
		return rhs.h == h;
	}


};



namespace std {
	template <>
	struct hash<Tet> {
		size_t operator()(const Tet& k) const {
			// Compute individual hash values for two data members and combine them using XOR and bit shifting
			auto h0 = hash<glm::dvec3>()(k.v0);
			auto h1 = hash<glm::dvec3>()(k.v1);
			auto h2 = hash<glm::dvec3>()(k.v2);
			auto h3 = hash<glm::dvec3>()(k.v3);

			std::list<size_t> hashes = { h0, h1, h2, h3 };
			hashes.sort();
			
			size_t seed = 0;
			for(auto& h : hashes) {
				hash_combine(seed, h);
			}

			return seed;

		}
	};
}

struct OffsetLookup {
	std::unordered_set<Tet> tets;
	std::unordered_set<glm::dvec3> parents;
	std::unordered_set<glm::dvec3> children;
};



class UniVolumePlanet{
public:

	std::unordered_set<glm::i8vec3> offset_lookups;
	std::unordered_map<glm::dvec3, std::shared_ptr<Diamond>> diamond_map;
	std::unordered_map<uint32_t, OffsetLookup> offset_map;
	std::unordered_set<glm::dvec3> diamond_types;

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

	
	UniVolumePlanet(double radius = 1.0, double maxHeightOffset = 0.1, double maxDepthOffset = 0.1, uint16_t divisions = 20, bool hasOcean = false);

	void Destroy();
	virtual ~UniVolumePlanet() { Destroy(); }

	void Initialize();
	void CreateGrid();
	void CreateOceanTriangles();
	void CreateQuads();
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
	uint16_t GridSize() { return m_Divisions; }
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

	uint16_t m_Divisions = 10;
	double m_GridZOffset = 0.0;

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

