#pragma once

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

#include "3dmaths.h"

#include <vulkan/vulkan.h>
#include "vks/VulkanBuffer.hpp"
#include "vks/VulkanModel.hpp"
#include "vks/VulkanTexture.hpp"
#include "vks/vulkanexamplebase.h"
#include <mutex>

#define VERTEX_BUFFER_BIND_ID 0
#define INSTANCE_BUFFER_BIND_ID 1
#define ENABLE_VALIDATION false
// todo: check if hardware supports sample number (or select max. supported)
#define SAMPLE_COUNT VK_SAMPLE_COUNT_8_BIT

namespace uni {
  namespace assets {
    class AssetManager;
  }
}
// forward declarations
class SceneManager;
class SceneRenderer;
class Input;
class AudioEngine;


class UniEngine final : public VulkanExampleBase {
 private:
  UniEngine(const UniEngine&) = delete;
  UniEngine& operator=(const UniEngine&) = delete;
  UniEngine(UniEngine&&) = delete;
  UniEngine& operator=(UniEngine&&) = delete;

  DWORD m_ID;

 public:
  UniEngine();
  ~UniEngine();
  static std::shared_ptr<UniEngine> GetInstance();
  static void Delete();

  void getEnabledFeatures() override;
  size_t getDynamicAlignment();
  void draw();
  void prepare() override;
  void render() override;
  void viewChanged() override;
  void OnUpdateUIOverlay(vks::UIOverlay* overlay) override;
  void ToggleWireframe();

  VkDevice GetDevice() { return device; }
  VkQueue GetQueue() { return queue; }
  VkPipelineCache GetPipelineCache() { return pipelineCache; }
  void Shutdown();
  bool m_debugDisplay = false;
  bool m_useWireframe = false;


 private:
  std::shared_ptr<SceneManager> m_SceneManager;
  std::shared_ptr<Input> m_InputManager;
  std::shared_ptr<AudioEngine> m_AudioManager;
  std::shared_ptr<uni::assets::AssetManager> m_AssetManager;

  bool m_CamPaused = false;
  float m_PlanetZOffset = 0;


 public:
  std::shared_ptr<SceneManager> GetSceneManager(){ return m_SceneManager; }
  std::shared_ptr<SceneRenderer> GetSceneRenderer();
  void windowResized() override;
  std::shared_ptr<Input> GetInputManager() { return m_InputManager; }
  std::shared_ptr<AudioEngine> GetAudioManager() { return m_AudioManager; }
  std::shared_ptr<uni::assets::AssetManager> GetAssetManager() { return m_AssetManager; }
  void SetupInput();

  void handleWMMessages(MSG& msg) override;

  void updateOverlay() override;
  void OnUpdateUserUIOverlay(vks::UIOverlay* overlay);

  VkRenderPass &GetRenderPass() { return renderPass; }

  std::vector<VkCommandBuffer> &GetCommandBuffers() { return drawCmdBuffers; }
  std::vector<VkFramebuffer> &GetFrameBuffers() {return frameBuffers; }
  void SetupOverlay();
  std::mutex m_QueueMutex;


};
