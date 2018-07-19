#pragma once

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