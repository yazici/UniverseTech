#pragma once

#include "3dmaths.h"
#include "fmod_studio.hpp"
#include "fmod.hpp"
#include <string>
#include <map>
#include <vector>
#include <math.h>
#include <iostream>

using namespace std;

struct Implementation {
  Implementation();
  ~Implementation();

  void Update();

  FMOD::Studio::System* mpStudioSystem;
  FMOD::System* mpSystem;

  int mnNextChannelId = 0;

  using SoundMap = map<string, FMOD::Sound*>;
  using ChannelMap = map<int, FMOD::Channel*>;
  using EventMap = map<string, FMOD::Studio::EventInstance*>;
  using BankMap = map<string, FMOD::Studio::Bank*>;

  BankMap mBanks;
  EventMap mEvents;
  SoundMap mSounds;
  ChannelMap mChannels;
};


class UniAudioEngine {
public:
  static shared_ptr<Implementation> GetImplementation();
  static void Init();
  static void Update();
  static void Shutdown();
  static int ErrorCheck(FMOD_RESULT result);

  void LoadBank(const string& strBankName, FMOD_STUDIO_LOAD_BANK_FLAGS flags);
  void LoadEvent(const string& strEventName);
  void LoadSound(const string& strSoundName,
    bool b3d = true,
    bool bLooping = false,
    bool bStream = false);
  void UnLoadSound(const string& strSoundName);
  void Set3dListenerAndOrientation(const glm::vec3& vPos, const glm::vec3& vUp, const glm::vec3& vForward, const glm::vec3& vVelocity = { 0, 0, 0 });
  int PlaySoundFile(const string& strSoundName,
    const glm::vec3& vPosition = { 0, 0, 0 },
    float fVolumedB = 0.0f);
  void PlayEvent(const string& strEventName);
  void StopChannel(int nChannelId);
  void StopEvent(const string& strEventName, bool bImmediate = false);
  void GetEventParameter(const string& strEventName,
    const string& strEventParameter,
    float* parameter);
  void SetEventParameter(const string& strEventName,
    const string& strParameterName,
    float fValue);
  void StopAllChannels();
  void SetChannel3dPosition(int nChannelId, const glm::vec3& vPosition);
  void SetChannelVolume(int nChannelId, float fVolumedB);
  bool IsPlaying(int nChannelId) const;
  bool IsEventPlaying(const string& strEventName) const;
  float dbToVolume(float db);
  float VolumeTodB(float volume);
  FMOD_VECTOR VectorToFmod(const glm::vec3& vPosition);
};
