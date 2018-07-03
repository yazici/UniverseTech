#pragma once
#include <gainput/gainput.h>
#include <memory>
#include <functional>
#include <map>
#include "vks/VulkanAndroid.h"
#include <vector>
#include <utility>

class UniInput;

class UniMappedButtonListener : public gainput::MappedInputListener {
public:
	UniMappedButtonListener(int index, UniInput& input) : index_(index), m_InputManager(input) {}

	bool OnUserButtonBool(gainput::UserButtonId button, bool oldValue, bool newValue) override;
	bool OnUserButtonFloat(gainput::UserButtonId button, float oldValue, float newValue) override;

	int GetPriority() const override {
		return index_;
	}
private:
	int index_;
	UniInput& m_InputManager;
};


class UniInput {

public:

	enum Button {
		ButtonPause,
		ButtonQuit,
		ButtonToggleUI,
		ButtonClick,
		ButtonRightClick,
		PointerX,
		PointerY,
		AxisPitch,
		AxisRoll,
		AxisYaw,
		AxisThrust,
		AxisReverse,
		AxisStrafe,
		AxisAscend
	};

	struct PointerPos {
		float X = 0.0f;
		float Y = 0.0f;
	};

	UniInput();
	~UniInput();
	gainput::InputManager m_InputManager;
	std::unique_ptr<gainput::InputMap> m_InputMap;
	std::unique_ptr<UniMappedButtonListener> m_Listener;
	std::map<Button, std::vector<std::function<void(bool)>>> m_ButtonCallbacks;
	std::map<Button, std::vector<std::function<void(float)>>> m_AxisCallbacks;
	std::vector<std::function<void(float, float)>> m_PointerPosCallbacks;
	std::map<Button, std::vector<std::function<void(float, float)>>> m_FloatButtonCallbacks;
	std::map<Button, std::vector<std::function<void(bool, bool)>>> m_BoolButtonCallbacks;
	std::map<Button, std::vector<std::function<void()>>> m_KeyDownCallbacks;
	std::map<Button, std::vector<std::function<void()>>> m_KeyUpCallbacks;

	void Initialize(int height, int width);

	void Tick();

	void HandleWM(MSG& msg);

	void RegisterButtonCallback(Button button, std::function<void(bool)>func);
	void HandleButton(Button button);

	void RegisterAxisCallback(Button axis, std::function<void(float)>func);
	void RegisterFloatCallback(Button button, std::function<void(float, float)>func);
	void RegisterBoolCallback(Button button, std::function<void(bool, bool)>func);
	void OnPress(Button button, std::function<void()>func);
	void OnRelease(Button button, std::function<void()>func);
	void RegisterPointerPosCallback(std::function<void(float, float)>func);
	void HandleBoolCallback(gainput::UserButtonId button, bool oldValue, bool newValue);
	void HandleFloatCallback(gainput::UserButtonId button, float oldValue, float newValue);
	PointerPos GetPointerXY();

	bool GetButtonState(Button button);
};

