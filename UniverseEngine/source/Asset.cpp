#include "Asset.h"
#include "UniEngine.h"
#include "AudioEngine.h"

using namespace uni::assets;

void UniAssetAudio::Destroy()
{
  UniEngine::GetInstance()->GetAudioManager()->UnLoadSound(m_sourceFile);
}