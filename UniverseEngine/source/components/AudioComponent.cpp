#include "AudioComponent.h"
#include "../UniEngine.h"
#include "../UniAssetManager.h"
#include "../UniAsset.h"

AudioComponent::AudioComponent(std::string path, bool is3d)
{
  auto asset = UniEngine::GetInstance()->AssetManager()->GetAsset<UniAssetAudio>(path);

  m_filename = asset->m_sourceFile;
  m_is3d = is3d;

}
