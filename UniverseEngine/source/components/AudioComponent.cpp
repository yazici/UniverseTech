#include "AudioComponent.h"
#include "../UniEngine.h"
#include "../AssetManager.h"
#include "../Asset.h"

using namespace uni::assets;

uni::components::AudioComponent::AudioComponent(std::string path)
{
  auto asset = UniEngine::GetInstance()->GetAssetManager()->GetAsset<UniAssetAudio>(path);

  m_filename = asset->m_sourceFile;
  m_is3d = asset->m_is3d;
  m_volume = asset->m_Volume;
  m_isLooping = asset->m_isLooping;

}
