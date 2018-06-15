#pragma once
#include <glm/glm.hpp>
#include <vector>

class ShaderData;
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
	Patch(uint16_t levels = 5);
	~Patch();

	void SetPlanet(UniBody* pPlanet) { m_pPlanet = pPlanet; }

	uint32_t GetVertexCount() { return static_cast<uint32_t>(m_Vertices.size()); }

	void Init();
	void GenerateGeometry(uint16_t levels);
	void BindInstances(std::vector<PatchInstance> &instances);
	void UploadDistanceLUT(std::vector<float> &distances);
	void Draw();
private:
	std::vector<PatchVertex>m_Vertices;
	std::vector<uint32_t>m_Indices;

	UniBody *m_pPlanet = nullptr;

	uint32_t m_NumInstances = 0;

	uint16_t m_Levels;
	uint32_t m_RC;

	////OpenGl stuff
	//GLuint m_VAO;
	//GLuint m_VBO;
	//GLuint m_EBO;
	//GLuint m_VBOInstance;

	//ShaderData *m_pPatchShader = nullptr;

	//GLint m_uModel;
	//GLint m_uViewProj;

	//GLint m_uMaxHeight;

	//GLint m_uCamPos;
	//GLint m_uRadius;
	//float m_MorphRange = 0.5f;
	//GLint m_uMorphRange;

	//GLint m_uDelta;

	////shading
	//vec3 m_Ambient = vec3(0.05f, 0.05f, 0.08f);
	//GLint m_uAmbient;
};