#pragma once
#include <gainput/gainput.h>
#include <memory>
#include <functional>
#include <map>
#include "vks/VulkanAndroid.h"
#include <vector>
#include <utility>


class UniInput {

public:

	enum Button {
		ButtonPause,
		ButtonQuit,
		ButtonToggleUI,
		ButtonClick,
		ButtonRightClick,
		PointerX,
		PointerY
	};

	struct PointerPos {
		float X = 0.0f;
		float Y = 0.0f;
	};



	UniInput();
	~UniInput();
	gainput::InputManager m_InputManager;
	std::unique_ptr<gainput::InputMap> m_InputMap;

	std::map<Button, std::vector<std::function<void(bool)>>> m_ButtonCallbacks;
	std::vector<std::function<void(float, float)>> m_PointerPosCallbacks;

	void Initialize(int height, int width);

	void Tick();

	void HandleWM(MSG& msg);

	void RegisterButtonCallback(Button button, std::function<void(bool)>func);
	void HandleButton(Button button);

	void RegisterPointerPosCallback(std::function<void(float, float)>func);
	PointerPos GetPointerXY();

	bool GetButtonState(Button button);
};

