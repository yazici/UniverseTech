#include "UniInput.h"

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

	m_InputMap->MapBool(ButtonToggleUI, keyboardId, gainput::KeyF1);
	m_InputMap->MapBool(ButtonToggleUI, keyboardId, gainput::KeyU);

	m_InputMap->MapBool(ButtonQuit, keyboardId, gainput::KeyEscape);

	m_InputMap->MapBool(ButtonPause, keyboardId, gainput::KeyP);

	m_InputMap->MapFloat(PointerX, mouseId, gainput::MouseAxisX);
	m_InputMap->MapFloat(PointerY, mouseId, gainput::MouseAxisY);
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
	m_ButtonCallbacks[button].push_back(func);
}

void UniInput::HandleButton(Button button)
{
	auto callbacks = m_ButtonCallbacks[button];

	for (const auto& cb : callbacks) {
		cb(true);
	}
}

void UniInput::RegisterPointerPosCallback(std::function<void(float, float)>func) {
	m_PointerPosCallbacks.push_back(func);
}

