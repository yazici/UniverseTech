#include "Input.h"
#include <GainputMapFilters.h>
#include <ostream>
#include <iostream>
#include "UniEngine.h"

Input::Input() {

}

Input::~Input() {

}

void Input::Initialize() {

  auto height = UniEngine::GetInstance()->height;
  auto width = UniEngine::GetInstance()->width;

  m_InputManager.SetDisplaySize(width, height);

  m_InputManager.CreateDevice<gainput::InputDeviceKeyboard>(gainput::InputDevice::DV_RAW);
  const gainput::DeviceId keyboardId = m_InputManager.CreateDevice<gainput::InputDeviceKeyboard>();

  m_InputManager.CreateDevice<gainput::InputDeviceMouse>(gainput::InputDevice::DV_RAW);
  const gainput::DeviceId mouseId = m_InputManager.CreateDevice<gainput::InputDeviceMouse>();

  //m_InputManager.CreateDevice<gainput::InputDevicePad>(gainput::InputDevice::DV_RAW);
  const gainput::DeviceId padId = m_InputManager.CreateDevice<gainput::InputDevicePad>();

  //m_InputManager.CreateDevice<gainput::InputDeviceTouch>(gainput::InputDevice::DV_RAW);
  const gainput::DeviceId touchId = m_InputManager.CreateDevice<gainput::InputDeviceTouch>();

  m_InputMap = std::make_unique<gainput::InputMap>(m_InputManager, "playermap");

  // map for per-frame buttons

  m_InputMap->MapBool(ButtonClick, keyboardId, gainput::KeyReturn);
  m_InputMap->MapBool(ButtonClick, mouseId, gainput::MouseButtonLeft);
  m_InputMap->MapBool(ButtonClick, padId, gainput::PadButtonA);
  m_InputMap->MapBool(ButtonClick, touchId, gainput::Touch0Down);

  m_InputMap->MapBool(ButtonRightClick, mouseId, gainput::MouseButtonRight);
  m_InputMap->MapBool(ButtonRightClick, padId, gainput::PadButtonB);

  m_InputMap->MapBool(ButtonToggleUI, keyboardId, gainput::KeyF1);
  m_InputMap->MapBool(ButtonToggleUI, keyboardId, gainput::KeyU);

  m_InputMap->MapBool(ButtonQuit, keyboardId, gainput::KeyEscape);
  m_InputMap->MapBool(ButtonExperiment, keyboardId, gainput::KeyX);

  m_InputMap->MapBool(ButtonPause, keyboardId, gainput::KeyP);
  m_InputMap->MapBool(ButtonPause, padId, gainput::PadButtonStart);

  m_InputMap->MapFloat(PointerX, mouseId, gainput::MouseAxisX);
  m_InputMap->MapFloat(PointerY, mouseId, gainput::MouseAxisY);

  m_InputMap->MapFloat(AxisThrust, padId, gainput::PadButtonAxis5, 0.f, 1.f);
  m_InputMap->MapFloat(AxisReverse, padId, gainput::PadButtonAxis4, 0.f, 1.f);

  m_InputMap->MapFloat(AxisYaw, padId, gainput::PadButtonRightStickX);
  m_InputMap->MapFloat(AxisPitch, padId, gainput::PadButtonRightStickY);
  m_InputMap->MapBool(ButtonRollLeft, padId, gainput::PadButtonL1);
  m_InputMap->MapBool(ButtonRollRight, padId, gainput::PadButtonR1);
  m_InputMap->MapFloat(AxisStrafe, padId, gainput::PadButtonLeftStickX);
  m_InputMap->MapFloat(AxisAscend, padId, gainput::PadButtonLeftStickY);

  m_InputMap->MapBool(ButtonBoostUp, padId, gainput::PadButtonUp);
  m_InputMap->MapBool(ButtonBoostDown, padId, gainput::PadButtonDown);


  m_Listener = std::make_unique<MappedButtonListener>(1, *this);
  gainput::ListenerId buttonListenerId = m_InputMap->AddListener(m_Listener.get());

  /*m_DeviceListener = std::make_unique<UniDeviceButtonListener>(m_InputManager, 0);
  m_InputManager.AddListener(m_DeviceListener.get());*/
}

Input::PointerPos Input::GetPointerXY() {
  return { m_InputMap->GetFloat(PointerX) * m_InputManager.GetDisplayWidth(), m_InputMap->GetFloat(PointerY) * m_InputManager.GetDisplayHeight() };
}

bool Input::GetButtonState(Button button) {
  return m_InputMap->GetBool(button);
}

void Input::Tick() {
  m_InputManager.Update();

  for (auto key : m_ButtonCallbacks) {
    if (m_InputMap->GetBoolWasDown(key.first)) {
      HandleButton(key.first);
    }
  }

  if (m_InputMap->GetFloatDelta(PointerX) != 0.0f || m_InputMap->GetFloatDelta(PointerY) != 0.0f) {
    for (const auto& cb : m_PointerPosCallbacks) {
      auto pos = GetPointerXY();
      cb(pos.X, pos.Y);
    }
  }


}

void Input::HandleWM(MSG& msg) {
  m_InputManager.HandleMessage(msg);
}

void Input::RegisterButtonCallback(Button button, std::function<void(bool)>func)
{
  m_ButtonCallbacks[button].emplace_back(func);
}

void Input::HandleButton(Button button)
{
  auto callbacks = m_ButtonCallbacks[button];

  for (const auto& cb : callbacks) {
    cb(true);
  }
}

void Input::RegisterAxisCallback(Button axis, std::function<void(float)>func) {
  m_AxisCallbacks[axis].emplace_back(func);
}

void Input::RegisterFloatCallback(Button button, std::function<void(float, float)>func) {
  m_FloatButtonCallbacks[button].emplace_back(func);
}

void Input::RegisterBoolCallback(Button button, std::function<void(bool, bool)>func) {
  m_BoolButtonCallbacks[button].emplace_back(func);
}

void Input::OnPress(Button button, std::function<void()>func) {
  m_KeyDownCallbacks[button].emplace_back(func);
}

void Input::OnRelease(Button button, std::function<void()>func) {
  m_KeyUpCallbacks[button].emplace_back(func);
}

void Input::RegisterPointerPosCallback(std::function<void(float, float)>func) {
  m_PointerPosCallbacks.emplace_back(func);
}

void Input::HandleBoolCallback(gainput::UserButtonId button, bool oldValue, bool newValue) {
  auto b = static_cast<Button>(button);
  auto callbacks = m_BoolButtonCallbacks[b];
  for (const auto& cb : callbacks) {
    cb(oldValue, newValue);
  }

  if (newValue && !oldValue) {
    auto kdCallbacks = m_KeyDownCallbacks[b];
    for (const auto& cb : kdCallbacks) {
      cb();
    }
  }

  if (oldValue && !newValue) {
    auto kuCallbacks = m_KeyUpCallbacks[b];
    for (const auto& cb : kuCallbacks) {
      cb();
    }
  }

}


void Input::HandleFloatCallback(gainput::UserButtonId button, float oldValue, float newValue) {
  auto callbacks = m_FloatButtonCallbacks[static_cast<Button>(button)];
  for (const auto& cb : callbacks) {
    //std::cout << "Handling callback for button: " << button << ", new value: " << newValue << std::endl;
    cb(oldValue, newValue);
  }
}


bool MappedButtonListener::OnUserButtonBool(gainput::UserButtonId button, bool oldValue, bool newValue) {

  m_InputManager.HandleBoolCallback(button, oldValue, newValue);
  return true;
}

bool MappedButtonListener::OnUserButtonFloat(gainput::UserButtonId button, float oldValue, float newValue) {
  m_InputManager.HandleFloatCallback(button, oldValue, newValue);
  return true;
}

bool DeviceButtonListener::OnDeviceButtonBool(gainput::DeviceId deviceId, gainput::DeviceButtonId deviceButton, bool oldValue, bool newValue) {
  const gainput::InputDevice* device = manager_.GetDevice(deviceId);
  char buttonName[64] = "";
  device->GetButtonName(deviceButton, buttonName, 64);
  std::cout << "Got button press: " << buttonName << ", value: " << newValue << std::endl;

  return false;
}
