#pragma once

#include "UniSceneObject.h"
#include "Triangulator.hpp"
#include "vks/VulkanTexture.hpp"

class UniBody :
	public UniSceneObject {
public:
	UniBody(double radius = 10.f);
	~UniBody();

	double GetRadius() { return m_Radius;  }
	double GetMaxHeight() { return m_MaxOffset; }

	double m_Radius = 10.0f;
	double m_MaxOffset = 1.f;
	
	double GetHeightOffsetAt(const glm::dvec3 pos) { return 0.0; }
	double GetHeightAt(const glm::dvec3 pos) { return m_Radius + GetHeightOffsetAt(pos); }

	void LoadTextures(vks::VulkanDevice *device, VkQueue copyQueue);

	uint32_t GetVertexCount();

	Triangulator* GetTriangulator() { return m_pTriangulator; }

	Triangulator* m_pTriangulator = nullptr;
	Patch* m_pPatch = nullptr;

	virtual void Initialize();
	virtual void Update();
	virtual void Draw();
	// virtual void Load() = 0;

	std::string m_HeightMapPath;
	std::string m_HeightDetailPath;
	std::string m_TexturePath;
	std::string m_Detail1Path;
	std::string m_Detail2Path;

	vks::Texture2D m_HeightMap;
	vks::Texture2D m_HeightDetail;
	vks::Texture2D m_Texture;
	vks::Texture2D m_Detail1;
	vks::Texture2D m_Detail2;

};

