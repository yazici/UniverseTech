#pragma once
#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

struct LightComponent {
	float radius;
	glm::vec4 color;
	bool enabled = true;
	LightComponent(float r = 1.0f, glm::vec4 c = glm::vec4(1.0f), bool on = true) : radius(r), color(c), enabled(on) {}
};
