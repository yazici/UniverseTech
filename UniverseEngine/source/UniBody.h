#pragma once
#include "UniSceneObject.h"
#include "Triangulator.hpp"

class UniBody :
	public UniSceneObject {
public:
	UniBody();
	~UniBody();

	double GetRadius() { return m_Radius;  }
	double GetMaxHeight() { return m_MaxOffset; }

	double m_Radius = 10.0f;
	double m_MaxOffset = 1.f;
	
	double GetHeightOffsetAt(const glm::dvec3 pos) { return 0.0; }
	double GetHeightAt(const glm::dvec3 pos) { return m_Radius + GetHeightOffsetAt(pos); }

	uint32_t GetVertexCount();

	Triangulator* GetTriangulator() { return m_pTriangulator; }

	Triangulator* m_pTriangulator = nullptr;
	Patch* m_pPatch = nullptr;

	virtual void Initialize();
	virtual void Update();
	virtual void Draw();
	// virtual void Load() = 0;

};

