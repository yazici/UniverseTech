#pragma once

#include <iostream>
#include <memory>
#include "../ECS.h"
#include "../3dmaths.h"

class UniSceneObject;

class PhysicsComponent {
public:
	PhysicsComponent() = default;
	~PhysicsComponent() = default;

	PhysicsComponent(double mass) : m_Mass(mass) {}
	PhysicsComponent(double radius, double density) {
		m_Radius = radius;
		m_Mass = (4.0 / 3.0) * M_PI * pow(radius, 3.0) * density;
		std::cout << "Radius: " << radius << ", Density: " << density << ". Setting mass: " << m_Mass << "kg" << std::endl;
	}

	PhysicsComponent(double radius, double density, bool isStatic = false) {
		m_IsStatic = isStatic;
		m_Radius = radius;
		m_Mass = (4.0 / 3.0) * M_PI * pow(radius, 3.0) * density;
		std::cout << "Radius: " << radius << ", Density: " << density << ". Setting mass: " << m_Mass << "kg" << std::endl;
	}

	std::shared_ptr<UniSceneObject> m_Parent;

	double m_Mass = 0.0;
	bool m_IsStatic = false;

	glm::dvec3 m_Velocity = glm::dvec3(0.0); // in world space
	glm::quat m_Torque = glm::quat(); // in world space

	glm::dvec3 m_CentreOfMass = glm::dvec3(0);
	double m_Radius = 0.0;

	double m_Drag = 0.0;
	double m_AngularDrag = 0.0;

	void AddForce(glm::dvec3 force);
	void AddForceAt(glm::dvec3 force, glm::dvec3 pos);
	void AddTorque(glm::dvec3 torque);
	void AddTorque(const glm::vec3 &axis, float angle);
	void FullStop() { m_Velocity = glm::dvec3(0); m_Torque = glm::quat(); }

};

