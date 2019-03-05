#pragma once

#include <string>
#include <memory>
#include "components/LightComponent.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class UniAsset;

class UniImporter
{
public:
  UniImporter();
  virtual ~UniImporter() = default;
  virtual std::shared_ptr<UniAsset> Import(json data) = 0;
  virtual std::shared_ptr<UniAsset> Import(json data, json settings) = 0;

protected:
  std::string m_name;
};


class UniLightImporter : public UniImporter {
public:
  UniLightImporter();
  std::shared_ptr<UniAsset> Import(json data) override;
  std::shared_ptr<UniAsset> Import(json data, json settings) override;
};