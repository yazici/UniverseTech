#pragma once
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <vector>
#include "Geometry.hpp"

class Camera;

enum class VolumeCheck {
	OUTSIDE,
	INTERSECT,
	CONTAINS
};

struct FrustumCorners {
	//Utility to transform frustum into any objects space
	//Useful for complex frustum culling operations
	//near plane
	glm::vec3 na;
	glm::vec3 nb;
	glm::vec3 nc;
	glm::vec3 nd;
	//far plane
	glm::vec3 fa;
	glm::vec3 fb;
	glm::vec3 fc;
	glm::vec3 fd;

	void Transform(glm::mat4 space) {
		//move corners of the near plane
		na = (space*glm::vec4(na, 0));
		nb = (space*glm::vec4(nb, 0));
		nc = (space*glm::vec4(nc, 0));
		nd = (space*glm::vec4(nd, 0));
		//move corners of the far plane
		fa = (space*glm::vec4(fa, 0));
		fb = (space*glm::vec4(fb, 0));
		fc = (space*glm::vec4(fc, 0));
		fd = (space*glm::vec4(fd, 0));
	}

};

class UniFrustum {
public:
	UniFrustum();
	~UniFrustum();

	void Update();

	// void SetToCamera(CameraComponent* pCamera);
	void SetCullTransform(glm::mat4 objectWorld);

	void SetToCamera(Camera* pCamera);
	VolumeCheck ContainsPoint(const glm::vec3 &point) const;
	VolumeCheck ContainsSphere(const Sphere &sphere) const;
	VolumeCheck ContainsTriangle(glm::vec3 &a, glm::vec3 &b, glm::vec3 &c);
	VolumeCheck ContainsTriVolume(glm::vec3 &a, glm::vec3 &b, glm::vec3 &c, float height);

	const glm::vec3 &GetPositionOS() { return m_PositionObject; }
	const float GetFOV() { return m_FOV; }
	const float GetRadInvFOV() { return m_RadInvFOV; }

	FrustumCorners GetCorners() { return m_Corners; }

private:
	//transform to the culled objects object space and back to world space
	glm::mat4 m_CullWorld, m_CullInverse;

	//stuff in the culled objects object space
	std::vector<Plane> m_Planes;
	FrustumCorners m_Corners;
	glm::vec3 m_PositionObject;

	float m_RadInvFOV;

	//camera parameters for locking
	glm::vec3 m_Position;
	glm::vec3 m_Forward;
	glm::vec3 m_Up;
	glm::vec3 m_Right;
	float m_NearPlane, m_FarPlane, m_FOV;
};