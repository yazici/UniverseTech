#pragma once
#include <gainput/gainput.h>
#include <memory>
#include <functional>
#include <map>
#include "vks/VulkanAndroid.h"
#include <vector>


class UniInput {

public:

	enum Button {
		ButtonPause,
		ButtonQuit,
		ButtonToggleUI,
		ButtonClick,
		ButtonRightClick,
	};



	UniInput();
	~UniInput();
	gainput::InputManager m_InputManager;
	std::unique_ptr<gainput::InputMap> m_InputMap;

	std::map<Button, std::vector<std::function<void(bool)>>> m_ButtonCallbacks;

	void Initialize(int height, int width);

	void Tick();

	void HandleWM(MSG& msg);

	void RegisterCallback(Button button, std::function<void(bool)>func);
	void HandleButton(Button button);
};

