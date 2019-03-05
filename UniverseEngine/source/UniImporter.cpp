#include "UniImporter.h"
#include "UniAsset.h"

UniImporter::UniImporter()
{
}


UniImporter::~UniImporter()
{
}

UniLightImporter::UniLightImporter()
{
}

std::shared_ptr<UniAsset> UniLightImporter::Import(json data)
{
  return std::make_shared<UniAsset>();
}

std::shared_ptr<UniAsset> UniLightImporter::Import(json data, json settings)
{
  return std::make_shared<UniAsset>();
}
