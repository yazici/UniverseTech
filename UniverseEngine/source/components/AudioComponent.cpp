#include "AudioComponent.h"
#include "../UniEngine.h"
#include "../UniAssetManager.h"
#include "../UniAsset.h"

AudioComponent::AudioComponent(std::string path)
{
  auto asset = UniEngine::GetInstance()->AssetManager()->GetAsset<UniAssetAudio>(path);

  m_filename = asset->m_sourceFile;
  m_is3d = asset->m_is3d;
  m_volume = asset->m_Volume;
  m_isLooping = asset->m_isLooping;

}
