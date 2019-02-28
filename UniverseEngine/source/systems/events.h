#pragma once
#include <vulkan/vulkan_core.h>

struct CameraPauseEvent {
	bool value = false;
};

struct InputEvent {
	int axis;
	float value;
};

struct PlanetZEvent {
	float value;
};

struct RenderEvent {
  VkCommandBuffer& cmdBuffer;
};