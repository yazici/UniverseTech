#include "InputManager.h"
#include <functional>


InputManager::InputManager() {}

InputManager::~InputManager() {}

void InputManager::SetupInput(GLFWwindow* window) {
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwGetCursorPos(window, &lastMouseX, &lastMouseY);
}

void InputManager::AddKeyHandler(int key, key_cb_t callback) {
	m_KeyCallbacks[key].push_back(callback);
}

void InputManager::ClearKeyHandler(int key /*= -1*/) {
	if(key == -1) {
		m_KeyCallbacks.clear();
	} else {
		m_KeyCallbacks[key].clear();
	}
}

void InputManager::AddMouseMoveHandler(mpos_cb_t callback) {
	m_MousePosCallbacks.push_back(callback);
}

void InputManager::ClearMouseMoveHandlers() {
	m_MousePosCallbacks.clear();

}

void InputManager::handleKeyboard(GLFWwindow* window, int key, int scancode, int action, int mods) {
	for(auto cb : m_KeyCallbacks[key]) {
		cb(key, scancode, action, mods);
	}
}

void InputManager::handleMousePos(GLFWwindow* window, double xpos, double ypos) {

	double relativeX = xpos - lastMouseX;
	double relativeY = ypos - lastMouseY;

	for(auto cb : m_MousePosCallbacks) {
		cb(relativeX, relativeY);
	}

	lastMouseX = xpos;
	lastMouseY = ypos;
}
