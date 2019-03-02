#include "UniAudioEngine.h"

Implementation::Implementation() {
  mnNextChannelId = 0;
  mpStudioSystem = nullptr;
  UniAudioEngine::ErrorCheck(FMOD::Studio::System::create(&mpStudioSystem));
  UniAudioEngine::ErrorCheck(mpStudioSystem->initialize(
    32, FMOD_STUDIO_INIT_LIVEUPDATE, FMOD_INIT_PROFILE_ENABLE, nullptr));

  mpSystem = nullptr;
  UniAudioEngine::ErrorCheck(mpStudioSystem->getLowLevelSystem(&mpSystem));
}

Implementation::~Implementation() {
  UniAudioEngine::ErrorCheck(mpStudioSystem->unloadAll());
  UniAudioEngine::ErrorCheck(mpStudioSystem->release());
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
  UniAudioEngine::ErrorCheck(mpStudioSystem->update());
}


shared_ptr<Implementation> UniAudioEngine::GetImplementation() {
  static std::shared_ptr<Implementation> instance(new Implementation);
  return instance;
}

void UniAudioEngine::Init() {

}

void UniAudioEngine::Update() {
  GetImplementation()->Update();
}

void UniAudioEngine::LoadSound(const std::string& strSoundName,
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
  UniAudioEngine::ErrorCheck(GetImplementation()->mpSystem->createSound(
    strSoundName.c_str(), eMode, nullptr, &pSound));
  if (pSound) {
    GetImplementation()->mSounds[strSoundName] = pSound;
  }
}

void UniAudioEngine::UnLoadSound(const std::string& strSoundName) {
  auto tFoundIt = GetImplementation()->mSounds.find(strSoundName);
  if (tFoundIt == GetImplementation()->mSounds.end())
    return;

  UniAudioEngine::ErrorCheck(tFoundIt->second->release());
  GetImplementation()->mSounds.erase(tFoundIt);
}

void UniAudioEngine::Set3dListenerAndOrientation(const glm::vec3 & vPos, const glm::vec3 & vUp, const glm::vec3 & vForward, const glm::vec3& vVelocity)
{
  FMOD_VECTOR position = VectorToFmod(vPos);
  FMOD_VECTOR up = VectorToFmod(vUp);
  FMOD_VECTOR forward = VectorToFmod(vForward);
  FMOD_VECTOR velocity = VectorToFmod({ 0, 0, 0 });
  UniAudioEngine::ErrorCheck(GetImplementation()->mpSystem->set3DListenerAttributes(0, &position, &velocity, &forward, &up));
}


int UniAudioEngine::PlaySoundFile(const string & strSoundName,
  const glm::vec3 & vPosition,
  float fVolumedB) {
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
  UniAudioEngine::ErrorCheck(GetImplementation()->mpSystem->playSound(
    tFoundIt->second, nullptr, true, &pChannel));
  if (pChannel) {
    FMOD_MODE currMode;
    tFoundIt->second->getMode(&currMode);
    if (currMode & FMOD_3D) {
      FMOD_VECTOR position = VectorToFmod(vPosition);
      UniAudioEngine::ErrorCheck(pChannel->set3DAttributes(&position, nullptr));
    }
    UniAudioEngine::ErrorCheck(pChannel->setVolume(dbToVolume(fVolumedB)));
    UniAudioEngine::ErrorCheck(pChannel->setPaused(false));
    GetImplementation()->mChannels[nChannelId] = pChannel;
  }
  return nChannelId;
}

void UniAudioEngine::SetChannel3dPosition(int nChannelId,
  const glm::vec3 & vPosition) {
  auto tFoundIt = GetImplementation()->mChannels.find(nChannelId);
  if (tFoundIt == GetImplementation()->mChannels.end())
    return;

  FMOD_VECTOR position = VectorToFmod(vPosition);
  UniAudioEngine::ErrorCheck(tFoundIt->second->set3DAttributes(&position, nullptr));
}

void UniAudioEngine::SetChannelVolume(int nChannelId, float fVolumedB) {
  auto tFoundIt = GetImplementation()->mChannels.find(nChannelId);
  if (tFoundIt == GetImplementation()->mChannels.end())
    return;

  UniAudioEngine::ErrorCheck(tFoundIt->second->setVolume(dbToVolume(fVolumedB)));
}

void UniAudioEngine::LoadBank(const std::string & strBankName,
  FMOD_STUDIO_LOAD_BANK_FLAGS flags) {
  auto tFoundIt = GetImplementation()->mBanks.find(strBankName);
  if (tFoundIt != GetImplementation()->mBanks.end())
    return;
  FMOD::Studio::Bank * pBank;
  UniAudioEngine::ErrorCheck(GetImplementation()->mpStudioSystem->loadBankFile(
    strBankName.c_str(), flags, &pBank));
  if (pBank) {
    GetImplementation()->mBanks[strBankName] = pBank;
  }
}

void UniAudioEngine::LoadEvent(const std::string & strEventName) {
  auto tFoundit = GetImplementation()->mEvents.find(strEventName);
  if (tFoundit != GetImplementation()->mEvents.end())
    return;
  FMOD::Studio::EventDescription * pEventDescription = nullptr;
  UniAudioEngine::ErrorCheck(GetImplementation()->mpStudioSystem->getEvent(
    strEventName.c_str(), &pEventDescription));
  if (pEventDescription) {
    FMOD::Studio::EventInstance* pEventInstance = nullptr;
    UniAudioEngine::ErrorCheck(
      pEventDescription->createInstance(&pEventInstance));
    if (pEventInstance) {
      GetImplementation()->mEvents[strEventName] = pEventInstance;
    }
  }
}

void UniAudioEngine::PlayEvent(const string & strEventName) {
  auto tFoundit = GetImplementation()->mEvents.find(strEventName);
  if (tFoundit == GetImplementation()->mEvents.end()) {
    LoadEvent(strEventName);
    tFoundit = GetImplementation()->mEvents.find(strEventName);
    if (tFoundit == GetImplementation()->mEvents.end())
      return;
  }
  tFoundit->second->start();
}

void UniAudioEngine::StopEvent(const string & strEventName, bool bImmediate) {
  auto tFoundIt = GetImplementation()->mEvents.find(strEventName);
  if (tFoundIt == GetImplementation()->mEvents.end())
    return;

  FMOD_STUDIO_STOP_MODE eMode;
  eMode =
    bImmediate ? FMOD_STUDIO_STOP_IMMEDIATE : FMOD_STUDIO_STOP_ALLOWFADEOUT;
  UniAudioEngine::ErrorCheck(tFoundIt->second->stop(eMode));
}

bool UniAudioEngine::IsEventPlaying(const string & strEventName) const {
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

void UniAudioEngine::GetEventParameter(const string & strEventName,
  const string & strParameterName,
  float* parameter) {
  auto tFoundIt = GetImplementation()->mEvents.find(strEventName);
  if (tFoundIt == GetImplementation()->mEvents.end())
    return;

  FMOD::Studio::ParameterInstance * pParameter = nullptr;
  UniAudioEngine::ErrorCheck(
    tFoundIt->second->getParameter(strParameterName.c_str(), &pParameter));
  UniAudioEngine::ErrorCheck(pParameter->getValue(parameter));
}

void UniAudioEngine::SetEventParameter(const string & strEventName,
  const string & strParameterName,
  float fValue) {
  auto tFoundIt = GetImplementation()->mEvents.find(strEventName);
  if (tFoundIt == GetImplementation()->mEvents.end())
    return;

  FMOD::Studio::ParameterInstance * pParameter = nullptr;
  UniAudioEngine::ErrorCheck(
    tFoundIt->second->getParameter(strParameterName.c_str(), &pParameter));
  UniAudioEngine::ErrorCheck(pParameter->setValue(fValue));
}


FMOD_VECTOR UniAudioEngine::VectorToFmod(const glm::vec3 & vPosition) {
  FMOD_VECTOR fVec;
  fVec.x = vPosition.x;
  fVec.y = vPosition.y;
  fVec.z = vPosition.z;
  return fVec;
}

float UniAudioEngine::dbToVolume(float dB) {
  return powf(10.0f, 0.05f * dB);
}

float UniAudioEngine::VolumeTodB(float volume) {
  return 20.0f* log10f(volume);
}

int UniAudioEngine::ErrorCheck(FMOD_RESULT result) {
  if (result != FMOD_OK) {
    cout << "FMOD ERROR " << result << endl;
    return 1;
  }
  // cout << "FMOD all good" << endl;
  return 0;
}

void UniAudioEngine::Shutdown() {
  GetImplementation().reset();
}