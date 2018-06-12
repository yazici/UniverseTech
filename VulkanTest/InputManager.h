#pragma once

#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H

#include "VulkanCommon.h"
#include <map>
#include <vector>
#include <functional>

typedef std::function<void(int, int, int, int)> key_cb_t;
typedef std::function<void(double, double)> mpos_cb_t;

class InputManager {
public:
	InputManager();
	~InputManager();

	void SetupInput(GLFWwindow* window);
	void AddKeyHandler(int key, key_cb_t callback);
	void ClearKeyHandler(int key = -1);
	void AddMouseMoveHandler(mpos_cb_t callback);
	void ClearMouseMoveHandlers();
	void handleKeyboard(GLFWwindow* window, int key, int scancode, int action, int mods);
	void handleMousePos(GLFWwindow* window, double xpos, double ypos);


private:
	std::map<int, std::vector<key_cb_t>> m_KeyCallbacks;
	std::vector<mpos_cb_t> m_MousePosCallbacks;

	double lastMouseX = 0.0;
	double lastMouseY = 0.0;
};

#endif