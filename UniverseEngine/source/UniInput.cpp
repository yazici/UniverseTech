#include "UniInput.h"
#include <GainputMapFilters.h>

UniInput::UniInput() {

}

UniInput::~UniInput() {

}

void UniInput::Initialize(int height, int width) {
	m_InputManager.SetDisplaySize(width, height);
	const gainput::DeviceId keyboardId = m_InputManager.CreateDevice<gainput::InputDeviceKeyboard>();
	const gainput::DeviceId mouseId = m_InputManager.CreateDevice<gainput::InputDeviceMouse>();
	const gainput::DeviceId padId = m_InputManager.CreateDevice<gainput::InputDevicePad>();
	const gainput::DeviceId touchId = m_InputManager.CreateDevice<gainput::InputDeviceTouch>();

	m_InputMap = std::make_unique<gainput::InputMap>(m_InputManager);
	m_InputMap->MapBool(ButtonClick, keyboardId, gainput::KeyReturn);
	m_InputMap->MapBool(ButtonClick, mouseId, gainput::MouseButtonLeft);
	m_InputMap->MapBool(ButtonClick, padId, gainput::PadButtonA);
	m_InputMap->MapBool(ButtonClick, touchId, gainput::Touch0Down);

	m_InputMap->MapBool(ButtonRightClick, mouseId, gainput::MouseButtonRight);
	m_InputMap->MapBool(ButtonRightClick, padId, gainput::PadButtonB);

	m_InputMap->MapBool(ButtonToggleUI, keyboardId, gainput::KeyF1);
	m_InputMap->MapBool(ButtonToggleUI, keyboardId, gainput::KeyU);

	m_InputMap->MapBool(ButtonQuit, keyboardId, gainput::KeyEscape);

	m_InputMap->MapBool(ButtonPause, keyboardId, gainput::KeyP);

	m_InputMap->MapFloat(PointerX, mouseId, gainput::MouseAxisX);
	m_InputMap->MapFloat(PointerY, mouseId, gainput::MouseAxisY);

	m_InputMap->MapFloat(AxisThrust, padId, gainput::PadButtonR2, 0.f, 1.f);
	m_InputMap->MapFloat(AxisThrust, padId, gainput::PadButtonL2, 0.f, 1.f, gainput::InvertSymmetricInput);
	m_InputMap->MapFloat(AxisStrafe, padId, gainput::PadButtonRight, 0.f, 1.f);
	m_InputMap->MapFloat(AxisStrafe, padId, gainput::PadButtonLeft, 0.f, 1.f, gainput::InvertSymmetricInput);


	m_InputMap->MapFloat(AxisYaw, padId, gainput::PadButtonLeftStickX);
	m_InputMap->MapFloat(AxisPitch, padId, gainput::PadButtonLeftStickY);
	m_InputMap->MapFloat(AxisRoll, padId, gainput::PadButtonRightStickX);
	m_InputMap->MapFloat(AxisAscend, padId, gainput::PadButtonRightStickY);
	

}

UniInput::PointerPos UniInput::GetPointerXY() {
	return { m_InputMap->GetFloat(PointerX) * m_InputManager.GetDisplayWidth(), m_InputMap->GetFloat(PointerY) * m_InputManager.GetDisplayHeight() };
}

bool UniInput::GetButtonState(Button button) {
	return m_InputMap->GetBool(button);
}

void UniInput::Tick() {
	m_InputManager.Update();

	for(auto key : m_ButtonCallbacks) {
		if(m_InputMap->GetBoolWasDown(key.first)) {
			HandleButton(key.first);
		}
	}

	if(m_InputMap->GetFloatDelta(PointerX) != 0.0f || m_InputMap->GetFloatDelta(PointerY) != 0.0f) {
		for(const auto& cb : m_PointerPosCallbacks) {
			auto pos = GetPointerXY();
			cb(pos.X, pos.Y);
		}
	}


}

void UniInput::HandleWM(MSG& msg) {
	m_InputManager.HandleMessage(msg);
}

void UniInput::RegisterButtonCallback(Button button, std::function<void(bool)>func)
{
	m_ButtonCallbacks[button].emplace_back(func);
}

void UniInput::HandleButton(Button button)
{
	auto callbacks = m_ButtonCallbacks[button];

	for (const auto& cb : callbacks) {
		cb(true);
	}
}

void UniInput::RegisterAxisCallback(Button axis, std::function<void(float)>func) {
	m_AxisCallbacks[axis].emplace_back(func);
}

void UniInput::RegisterPointerPosCallback(std::function<void(float, float)>func) {
	m_PointerPosCallbacks.emplace_back(func);
}

