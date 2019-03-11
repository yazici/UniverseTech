#pragma once

#include <iostream>
#include <memory>
#include "../ECS.h"
#include "../3dmaths.h"

namespace uni
{
  namespace scene
  {
    class SceneObject;
  }

  namespace components {
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

      std::shared_ptr<uni::scene::SceneObject> m_SceneObject;

      double m_Mass = 0.0;
      bool m_IsStatic = false;

      glm::dvec3 m_Velocity = glm::dvec3(0.0); // in world space
      glm::dvec3 m_AngularVelocity = glm::dvec3(0.0);

      glm::dvec3 m_CentreOfMass = glm::dvec3(0);
      double m_Radius = 0.0;

      double m_Drag = 0.0;
      double m_AngularDrag = 0.0;

      void AddForce(glm::dvec3 force);
      void AddForceAt(glm::dvec3 force, glm::dvec3 pos);
      void AddAngularVelocity(const glm::vec3 & angular);
      void FullStop() { m_Velocity = glm::dvec3(0); m_AngularVelocity = glm::dvec3(0); }
      void SetSceneObject(std::shared_ptr<uni::scene::SceneObject> so) { m_SceneObject = so; }
    };
  }
}

