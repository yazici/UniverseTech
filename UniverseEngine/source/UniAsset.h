#pragma once

#include <chrono>
#include <string>

class UniAssetImporter;


class UniAsset
{
public:
  UniAsset();
  ~UniAsset();
  void Destroy();
  bool Load();
protected:
  std::string m_sourceFile;
  uint64_t m_lastUpdate;
  uint64_t m_created;


public:
  bool m_isLoaded;
  //ImportSettings m_importSettings;
};

