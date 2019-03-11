#pragma once
#include "../3dmaths.h"



namespace uni
{
	namespace components
	{
		struct LightComponent {
			float radius;
			glm::vec4 color;
			bool enabled = true;
			LightComponent(float r = 1.0f, glm::vec4 c = glm::vec4(1.0f), bool on = true) : radius(r), color(c), enabled(on) {}
		};
	}
}
