#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "Patch.hpp"

class UniFrustum;
class UniBody;

enum TriNext {
	CULL,
	LEAF,
	SPLIT,
	SPLITCULL
};

struct Tri {
	Tri(glm::vec3 A, glm::vec3 B, glm::vec3 C, Tri* Parent, uint16_t Level)
		:a(A), b(B), c(C), parent(Parent), level(Level) {}

	Tri* parent = nullptr;

	Tri* c1 = nullptr;
	Tri* c2 = nullptr;
	Tri* c3 = nullptr;
	Tri* c4 = nullptr;

	TriNext state;

	uint16_t level;

	glm::vec3 a;
	glm::vec3 b;
	glm::vec3 c;
};

class Triangulator {
public:
	Triangulator(UniBody* pPlanet);
	~Triangulator();

	//Member functions
	void Init();
	bool Update();
	void GenerateGeometry();

	bool IsFrustumLocked() { return m_LockFrustum; }
	UniFrustum* GetFrustum() { return m_pFrustum; }
	uint32_t GetVertexCount() { return static_cast<uint32_t>(m_Positions.size()); }

private:
	friend class UniBody;

	void Precalculate();
	TriNext SplitHeuristic(glm::vec3 &a, glm::vec3 &b, glm::vec3 &c, uint16_t level, bool frustumCull);
	void RecursiveTriangle(glm::vec3 a, glm::vec3 b, glm::vec3 c, uint16_t level, bool frustumCull);

	//Triangulation paramenters
	float m_AllowedTriPx = 300.f;
	uint32_t m_MaxLevel = 15;

	std::vector<Tri> m_Icosahedron;
	std::vector<float> m_DistanceLUT;
	std::vector<float> m_TriLevelDotLUT;
	std::vector<float> m_HeightMultLUT;

	std::vector<Tri*> m_Leafs;

	UniBody* m_pPlanet = nullptr;
	UniFrustum* m_pFrustum = nullptr;
	bool m_LockFrustum = false;

	std::vector<PatchInstance> m_Positions;
};


