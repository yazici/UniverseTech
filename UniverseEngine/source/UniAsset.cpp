#include "UniAsset.h"
#include "UniEngine.h"
#include "UniAudioEngine.h"

void UniAssetAudio::Destroy()
{
  UniEngine::GetInstance()->AudioManager()->UnLoadSound(m_sourceFile);
}