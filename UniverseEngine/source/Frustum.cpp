#include "Frustum.hpp"


Frustum::Frustum() {
	m_Corners = FrustumCorners();
}
Frustum::~Frustum() {}

//create transforms to prevent transforming every triangle into world space
void Frustum::SetCullTransform(glm::mat4 objectWorld) {
	m_CullWorld = objectWorld;
	m_CullInverse = glm::inverse(objectWorld);
}

void Frustum::SetToCamera(ECS::ComponentHandle<CameraComponent> camera) {
	auto mat = camera->matrices.view;
	m_Position = glm::vec3(mat[3]);
	m_Up = glm::vec3(mat[1]);
	m_Forward = glm::vec3(mat[2]);
	m_Right = glm::vec3(mat[0]);
	m_NearPlane = camera->nearClip;
	m_FarPlane = camera->farClip;
	m_FOV = camera->fov;
	m_Aspect = camera->aspect;
}

void Frustum::Update() {
	//calculate generalized relative width and aspect ratio
	float normHalfWidth = tan(glm::radians(m_FOV));
	float aspectRatio = m_Aspect; // (float)UniEngine::GetInstance()->width / (float)UniEngine::GetInstance()->height;


	//calculate width and height for near and far plane
	float nearHW = normHalfWidth * m_NearPlane;
	float nearHH = nearHW / aspectRatio;
	float farHW = normHalfWidth * m_FarPlane;// *0.5f;
	float farHH = farHW / aspectRatio;

	//calculate near and far plane centers
	auto nCenter = m_Position + m_Forward * m_NearPlane;
	auto fCenter = m_Position + m_Forward * m_FarPlane *0.5f;

	//construct corners of the near plane in the culled objects world space
	m_Corners.na = nCenter + m_Up * nearHH - m_Right * nearHW;
	m_Corners.nb = nCenter + m_Up * nearHH + m_Right * nearHW;
	m_Corners.nc = nCenter - m_Up * nearHH - m_Right * nearHW;
	m_Corners.nd = nCenter - m_Up * nearHH + m_Right * nearHW;
	//construct corners of the far plane
	m_Corners.fa = fCenter + m_Up * farHH - m_Right * farHW;
	m_Corners.fb = fCenter + m_Up * farHH + m_Right * farHW;
	m_Corners.fc = fCenter - m_Up * farHH - m_Right * farHW;
	m_Corners.fd = fCenter - m_Up * farHH + m_Right * farHW;

	//float yFac = tanf( etm::radians(m_FOV) / 2 );
	//float xFac = yFac*WINDOW.GetAspectRatio();
	//vec3 nCenter = m_Position + m_Forward*m_NearPlane;
	//vec3 fCenter = m_Position + m_Forward*m_FarPlane;
	//vec3 nearHW = m_Right*m_FarPlane*xFac;
	//vec3 nearHH = m_Up*m_FarPlane*yFac;
	//vec3 farHW = m_Right*m_FarPlane*xFac;
	//vec3 farHH = m_Up*m_FarPlane*yFac;

	////construct corners of the near plane in the culled objects world space
	//m_Corners.na = nCenter + nearHH - nearHW;
	//m_Corners.nb = nCenter + nearHH + nearHW;
	//m_Corners.nc = nCenter - nearHH - nearHW;
	//m_Corners.nd = nCenter - nearHH + nearHW;
	////construct corners of the far plane
	//m_Corners.fa = fCenter + farHH - farHW;
	//m_Corners.fb = fCenter + farHH + farHW;
	//m_Corners.fc = fCenter - farHH - farHW;
	//m_Corners.fd = fCenter - farHH + farHW;
	m_Corners.Transform(m_CullInverse);

	m_PositionObject = m_CullInverse * glm::vec4(m_Position, 0);
	m_RadInvFOV = 1.f / glm::radians(m_FOV);

	//construct planes
	m_Planes.clear();
	//winding in an outside perspective so the cross product creates normals pointing inward
	m_Planes.emplace_back(m_Corners.na, m_Corners.nb, m_Corners.nc);//Near
	m_Planes.emplace_back(m_Corners.fb, m_Corners.fa, m_Corners.fd);//Far 
	m_Planes.emplace_back(m_Corners.fa, m_Corners.na, m_Corners.fc);//Left
	m_Planes.emplace_back(m_Corners.nb, m_Corners.fb, m_Corners.nd);//Right
	m_Planes.emplace_back(m_Corners.fa, m_Corners.fb, m_Corners.na);//Top
	m_Planes.emplace_back(m_Corners.nc, m_Corners.nd, m_Corners.fc);//Bottom
}

VolumeCheck Frustum::ContainsPoint(const glm::vec3 &point) const {
	for(auto plane : m_Planes) {
		if(glm::dot(plane.n, point - plane.d) < 0)return VolumeCheck::OUTSIDE;
	}
	return VolumeCheck::CONTAINS;
}
VolumeCheck Frustum::ContainsSphere(const Sphere &sphere) const {
	VolumeCheck ret = VolumeCheck::CONTAINS;
	for(auto plane : m_Planes) {
		float dist = glm::dot(plane.n, sphere.pos - plane.d);
		if(dist < -sphere.radius)return VolumeCheck::OUTSIDE;
		else if(dist < 0) ret = VolumeCheck::INTERSECT;
	}
	return ret;
}
//this method will treat triangles as intersecting even though they may be outside
//but it is faster then performing a proper intersection test with every plane
//and it does not reject triangles that are inside but with all corners outside
VolumeCheck Frustum::ContainsTriangle(glm::vec3 &a, glm::vec3 &b, glm::vec3 &c) {
	VolumeCheck ret = VolumeCheck::CONTAINS;
	for(auto plane : m_Planes) {
		char rejects = 0;
		if(glm::dot(plane.n, a - plane.d) < 0)rejects++;
		if(glm::dot(plane.n, b - plane.d) < 0)rejects++;
		if(glm::dot(plane.n, c - plane.d) < 0)rejects++;
		// if all three are outside a plane the triangle is outside the frustrum
		if(rejects >= 3)return VolumeCheck::OUTSIDE;
		// if at least one is outside the triangle intersects at least one plane
		else if(rejects > 0)ret = VolumeCheck::INTERSECT;
	}
	return ret;
}
//same as above but with a volume generated above the triangle
VolumeCheck Frustum::ContainsTriVolume(glm::vec3 &a, glm::vec3 &b, glm::vec3 &c, float height) {
	VolumeCheck ret = VolumeCheck::CONTAINS;
	for(auto plane : m_Planes) {
		char rejects = 0;
		if(glm::dot(plane.n, a - plane.d) < 0)rejects++;
		if(glm::dot(plane.n, b - plane.d) < 0)rejects++;
		if(glm::dot(plane.n, c - plane.d) < 0)rejects++;
		// if all three are outside a plane the triangle is outside the frustrum
		if(rejects >= 3) {
			if(glm::dot(plane.n, (a*height) - plane.d) < 0)rejects++;
			if(glm::dot(plane.n, (b*height) - plane.d) < 0)rejects++;
			if(glm::dot(plane.n, (c*height) - plane.d) < 0)rejects++;
			if(rejects >= 6)return VolumeCheck::OUTSIDE;
			else ret = VolumeCheck::INTERSECT;
		}
		// if at least one is outside the triangle intersects at least one plane
		else if(rejects > 0)ret = VolumeCheck::INTERSECT;
	}
	return ret;
}