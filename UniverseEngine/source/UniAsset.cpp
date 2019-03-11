#include "UniAsset.h"
#include "UniEngine.h"
#include "UniAudioEngine.h"

using namespace uni::assets;

void UniAssetAudio::Destroy()
{
  UniEngine::GetInstance()->AudioManager()->UnLoadSound(m_sourceFile);
}