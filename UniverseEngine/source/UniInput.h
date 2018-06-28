#pragma once
#include <gainput/gainput.h>


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
	gainput::InputMap m_InputMap;

	void Initialize(int height, int width);

	void Tick();

	void HandleWM(MSG& msg);
};

