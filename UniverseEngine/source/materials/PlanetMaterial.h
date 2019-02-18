#include "../UniMaterial.h"

class PlanetMaterial : public UniMaterial {
 public:
  struct SpecializationData {
    // Sets the displacement used in the tessellation shader
    bool isDisplaced = true;
  } m_SpecializationData;

  VkPipeline m_OceanPipeline;
  VkPipelineLayout m_OceanPipelineLayout;

  
  struct {
    uint32_t noiseLayers = 0;
  } m_PushConstants;


  virtual ~PlanetMaterial() { Destroy(); }


  uint32_t AddToCommandBuffer(VkCommandBuffer& cmdBuffer, uint32_t index) override;

  std::vector<std::shared_ptr<vks::Texture>> m_Textures;

  void SetOceanIndexCount(uint32_t count = 0) { m_OceanIndexCount = count; }
  void SetNoiseLayerCount(uint32_t c) { m_PushConstants.noiseLayers = c; }
  void Destroy() override;

 private:
  uint32_t m_OceanIndexCount;
  bool m_RenderOcean = false;

 public:
  PlanetMaterial() = default;
  PlanetMaterial(std::string name, bool hasOcean = false);
};


using PlanetMaterialFactory =
    Factory<std::string, std::shared_ptr<PlanetMaterial>>::Initializer<std::string,
                                                                    bool>;