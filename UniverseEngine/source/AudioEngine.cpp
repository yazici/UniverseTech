#include "AudioEngine.h"

using namespace uni::audio;

Implementation::Implementation() {
  mnNextChannelId = 0;
  mpStudioSystem = nullptr;
  AudioEngine::ErrorCheck(FMOD::Studio::System::create(&mpStudioSystem));
  AudioEngine::ErrorCheck(mpStudioSystem->initialize(
    32, FMOD_STUDIO_INIT_LIVEUPDATE, FMOD_INIT_PROFILE_ENABLE, nullptr));

  mpSystem = nullptr;
  AudioEngine::ErrorCheck(mpStudioSystem->getLowLevelSystem(&mpSystem));
}

Implementation::~Implementation() {
  AudioEngine::ErrorCheck(mpStudioSystem->unloadAll());
  AudioEngine::ErrorCheck(mpStudioSystem->release());
}


void Implementation::Update() {
  vector<ChannelMap::iterator> pStoppedChannels;
  for (auto it = mChannels.begin(), itEnd = mChannels.end(); it != itEnd;
    ++it) {
    bool bIsPlaying = false;
    it->second->isPlaying(&bIsPlaying);
    if (!bIsPlaying) {
      pStoppedChannels.push_back(it);
    }
  }
  for (auto& it : pStoppedChannels) {
    mChannels.erase(it);
  }
  AudioEngine::ErrorCheck(mpStudioSystem->update());
}


shared_ptr<Implementation> AudioEngine::GetImplementation() {
  static std::shared_ptr<Implementation> instance(new Implementation);
  return instance;
}

void AudioEngine::Init() {

}

void AudioEngine::Update() {
  GetImplementation()->Update();
}

void AudioEngine::LoadSound(const std::string& strSoundName,
  bool b3d,
  bool bLooping,
  bool bStream) {
  auto tFoundIt = GetImplementation()->mSounds.find(strSoundName);
  if (tFoundIt != GetImplementation()->mSounds.end())
    return;

  FMOD_MODE eMode = FMOD_DEFAULT;
  eMode |= b3d ? FMOD_3D : FMOD_2D;
  eMode |= bLooping ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF;
  eMode |= bStream ? FMOD_CREATESTREAM : FMOD_CREATECOMPRESSEDSAMPLE;

  FMOD::Sound * pSound = nullptr;
  AudioEngine::ErrorCheck(GetImplementation()->mpSystem->createSound(
    strSoundName.c_str(), eMode, nullptr, &pSound));
  if (pSound) {
    GetImplementation()->mSounds[strSoundName] = pSound;
  }
}

void AudioEngine::UnLoadSound(const std::string& strSoundName) {
  auto tFoundIt = GetImplementation()->mSounds.find(strSoundName);
  if (tFoundIt == GetImplementation()->mSounds.end())
    return;
  std::cout << "Unloading sound " << strSoundName << std::endl;
  AudioEngine::ErrorCheck(tFoundIt->second->release());
  GetImplementation()->mSounds.erase(tFoundIt);
}

void AudioEngine::Set3dListenerAndOrientation(const glm::vec3 & vPos, const glm::vec3 & vUp, const glm::vec3 & vForward, const glm::vec3& vVelocity)
{
  FMOD_VECTOR position = VectorToFmod(vPos);
  FMOD_VECTOR up = VectorToFmod(vUp);
  FMOD_VECTOR forward = VectorToFmod(vForward);
  FMOD_VECTOR velocity = VectorToFmod(vVelocity);
  AudioEngine::ErrorCheck(GetImplementation()->mpSystem->set3DListenerAttributes(0, &position, &velocity, &forward, &up));
}


int AudioEngine::PlaySoundFile(const string & strSoundName,
  const glm::vec3 & vPosition,
  float fVolumePercent) {
  int nChannelId = GetImplementation()->mnNextChannelId++;
  auto tFoundIt = GetImplementation()->mSounds.find(strSoundName);
  if (tFoundIt == GetImplementation()->mSounds.end()) {
    LoadSound(strSoundName);
    tFoundIt = GetImplementation()->mSounds.find(strSoundName);
    if (tFoundIt == GetImplementation()->mSounds.end()) {
      return nChannelId;
    }
  }
  FMOD::Channel* pChannel = nullptr;
  AudioEngine::ErrorCheck(GetImplementation()->mpSystem->playSound(
    tFoundIt->second, nullptr, true, &pChannel));
  if (pChannel) {
    FMOD_MODE currMode;
    tFoundIt->second->getMode(&currMode);
    if (currMode & FMOD_3D) {
      FMOD_VECTOR position = VectorToFmod(vPosition);
      AudioEngine::ErrorCheck(pChannel->set3DAttributes(&position, nullptr));
    }
    AudioEngine::ErrorCheck(pChannel->setVolume(fVolumePercent));
    AudioEngine::ErrorCheck(pChannel->setPaused(false));
    GetImplementation()->mChannels[nChannelId] = pChannel;
  }
  return nChannelId;
}

void AudioEngine::SetChannel3dPosition(int nChannelId,
  const glm::vec3 & vPosition) {
  auto tFoundIt = GetImplementation()->mChannels.find(nChannelId);
  if (tFoundIt == GetImplementation()->mChannels.end())
    return;

  FMOD_VECTOR position = VectorToFmod(vPosition);
  AudioEngine::ErrorCheck(tFoundIt->second->set3DAttributes(&position, nullptr));
}

void AudioEngine::SetChannelVolume(int nChannelId, float fVolumedB) {
  auto tFoundIt = GetImplementation()->mChannels.find(nChannelId);
  if (tFoundIt == GetImplementation()->mChannels.end())
    return;

  AudioEngine::ErrorCheck(tFoundIt->second->setVolume(dbToVolume(fVolumedB)));
}

bool AudioEngine::IsPlaying(int nChannelId) const
{
  auto tFoundIt = GetImplementation()->mChannels.find(nChannelId);
  if (tFoundIt == GetImplementation()->mChannels.end())
    return false;

  bool bIsPlaying = false;
  //std::cout << "Checking if channel " << nChannelId << " is playing" << std::endl;
  AudioEngine::ErrorCheck(GetImplementation()->mChannels[nChannelId]->isPlaying(&bIsPlaying));

  return bIsPlaying;

}

void AudioEngine::LoadBank(const std::string & strBankName,
  FMOD_STUDIO_LOAD_BANK_FLAGS flags) {
  auto tFoundIt = GetImplementation()->mBanks.find(strBankName);
  if (tFoundIt != GetImplementation()->mBanks.end())
    return;
  FMOD::Studio::Bank * pBank;
  AudioEngine::ErrorCheck(GetImplementation()->mpStudioSystem->loadBankFile(
    strBankName.c_str(), flags, &pBank));
  if (pBank) {
    GetImplementation()->mBanks[strBankName] = pBank;
  }
}

void AudioEngine::LoadEvent(const std::string & strEventName) {
  auto tFoundit = GetImplementation()->mEvents.find(strEventName);
  if (tFoundit != GetImplementation()->mEvents.end())
    return;
  FMOD::Studio::EventDescription * pEventDescription = nullptr;
  AudioEngine::ErrorCheck(GetImplementation()->mpStudioSystem->getEvent(
    strEventName.c_str(), &pEventDescription));
  if (pEventDescription) {
    FMOD::Studio::EventInstance* pEventInstance = nullptr;
    AudioEngine::ErrorCheck(
      pEventDescription->createInstance(&pEventInstance));
    if (pEventInstance) {
      GetImplementation()->mEvents[strEventName] = pEventInstance;
    }
  }
}

void AudioEngine::PlayEvent(const string & strEventName) {
  auto tFoundit = GetImplementation()->mEvents.find(strEventName);
  if (tFoundit == GetImplementation()->mEvents.end()) {
    LoadEvent(strEventName);
    tFoundit = GetImplementation()->mEvents.find(strEventName);
    if (tFoundit == GetImplementation()->mEvents.end())
      return;
  }
  tFoundit->second->start();
}

void AudioEngine::StopChannel(int nChannelId)
{
  auto tFoundIt = GetImplementation()->mChannels.find(nChannelId);
  if (tFoundIt == GetImplementation()->mChannels.end())
    return;

  if(IsPlaying(nChannelId)){
    AudioEngine::ErrorCheck(GetImplementation()->mChannels[nChannelId]->stop());
    Update();
  }

}

void AudioEngine::StopEvent(const string & strEventName, bool bImmediate) {
  auto tFoundIt = GetImplementation()->mEvents.find(strEventName);
  if (tFoundIt == GetImplementation()->mEvents.end())
    return;

  FMOD_STUDIO_STOP_MODE eMode;
  eMode =
    bImmediate ? FMOD_STUDIO_STOP_IMMEDIATE : FMOD_STUDIO_STOP_ALLOWFADEOUT;
  AudioEngine::ErrorCheck(tFoundIt->second->stop(eMode));
}

bool AudioEngine::IsEventPlaying(const string & strEventName) const {
  auto tFoundIt = GetImplementation()->mEvents.find(strEventName);
  if (tFoundIt == GetImplementation()->mEvents.end())
    return false;

  FMOD_STUDIO_PLAYBACK_STATE * state = nullptr;
  if (tFoundIt->second->getPlaybackState(state) ==
    FMOD_STUDIO_PLAYBACK_PLAYING) {
    return true;
  }
  return false;
}

void AudioEngine::GetEventParameter(const string & strEventName,
  const string & strParameterName,
  float* parameter) {
  auto tFoundIt = GetImplementation()->mEvents.find(strEventName);
  if (tFoundIt == GetImplementation()->mEvents.end())
    return;

  FMOD::Studio::ParameterInstance * pParameter = nullptr;
  AudioEngine::ErrorCheck(
    tFoundIt->second->getParameter(strParameterName.c_str(), &pParameter));
  AudioEngine::ErrorCheck(pParameter->getValue(parameter));
}

void AudioEngine::SetEventParameter(const string & strEventName,
  const string & strParameterName,
  float fValue) {
  auto tFoundIt = GetImplementation()->mEvents.find(strEventName);
  if (tFoundIt == GetImplementation()->mEvents.end())
    return;

  FMOD::Studio::ParameterInstance * pParameter = nullptr;
  AudioEngine::ErrorCheck(
    tFoundIt->second->getParameter(strParameterName.c_str(), &pParameter));
  AudioEngine::ErrorCheck(pParameter->setValue(fValue));
}


void AudioEngine::StopAllChannels()
{
  Update();

  for (auto& [k, channel] : GetImplementation()->mChannels) {
    StopChannel(k);
  }
}

FMOD_VECTOR AudioEngine::VectorToFmod(const glm::vec3 & vPosition) {
  FMOD_VECTOR fVec;
  fVec.x = vPosition.x;
  fVec.y = vPosition.y;
  fVec.z = vPosition.z;
  return fVec;
}

float AudioEngine::dbToVolume(float dB) {
  return powf(10.0f, 0.05f * dB);
}

float AudioEngine::VolumeTodB(float volume) {
  return 20.0f* log10f(volume);
}

int AudioEngine::ErrorCheck(FMOD_RESULT result) {
  if (result != FMOD_OK) {
    cout << "FMOD ERROR " << result << endl;
    return 1;
  }
  // cout << "FMOD all good" << endl;
  return 0;
}

void AudioEngine::Shutdown() {
  StopAllChannels();
}