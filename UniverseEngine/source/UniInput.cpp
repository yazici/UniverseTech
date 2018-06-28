#include "UniInput.h"

UniInput::UniInput() {}

UniInput::~UniInput() {}

void UniInput::Initialize(int height, int width) {
	m_InputManager.SetDisplaySize(width, height);
	const gainput::DeviceId keyboardId = m_InputManager.CreateDevice<gainput::InputDeviceKeyboard>();
	const gainput::DeviceId mouseId = m_InputManager.CreateDevice<gainput::InputDeviceMouse>();
	const gainput::DeviceId padId = m_InputManager.CreateDevice<gainput::InputDevicePad>();
	const gainput::DeviceId touchId = m_InputManager.CreateDevice<gainput::InputDeviceTouch>();

	gainput::InputMap m_InputMap(m_InputManager);
	m_InputMap.MapBool(ButtonClick, keyboardId, gainput::KeyReturn);
	m_InputMap.MapBool(ButtonClick, mouseId, gainput::MouseButtonLeft);
	m_InputMap.MapBool(ButtonClick, padId, gainput::PadButtonA);
	m_InputMap.MapBool(ButtonClick, touchId, gainput::Touch0Down);

	m_InputMap.MapBool(ButtonRightClick, mouseId, gainput::MouseButtonRight);

	m_InputMap.MapBool(ButtonToggleUI, keyboardId, gainput::KeyF1);
	m_InputMap.MapBool(ButtonToggleUI, keyboardId, gainput::KeyU);

	m_InputMap.MapBool(ButtonQuit, keyboardId, gainput::KeyEscape);

	m_InputMap.MapBool(ButtonPause, keyboardId, gainput::KeyP);
}

void UniInput::Tick() {
	m_InputManager.Update();
}

void UniInput::HandleWM(MSG& msg) {
	m_InputManager.HandleMessage(msg);
}
