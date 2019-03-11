
#include "UniEngine.h"
#include <assert.h>
#include <algorithm>
#include "AudioEngine.h"
#include "Input.h"
#include "Material.h"
#include "SceneManager.h"
#include "SceneRenderer.h"
#include "AssetManager.h"
#include "components/Components.h"
#include "systems/events.h"
#include "vks/VulkanTools.h"

#define ENABLE_VALIDATION true

void UniEngine::Shutdown() {
  std::cout << "Shutting down..." << std::endl;

  GetSceneManager()->Shutdown();
  GetAudioManager()->Shutdown();
  GetAssetManager()->Shutdown();



  m_SceneManager.reset();
  m_InputManager.reset();
  m_AudioManager.reset();
  m_AssetManager.reset();
}

UniEngine::~UniEngine() {
  // Shutdown();
  std::cout << "Engine " << m_ID << " is being deleted!" << std::endl;
}

UniEngine::UniEngine() : VulkanExampleBase(ENABLE_VALIDATION) {
  m_AssetManager = std::make_shared<uni::assets::AssetManager>(getAssetPath() + "assets");
  m_SceneManager = std::make_shared<uni::scene::SceneManager>();
  m_AudioManager = std::make_shared<uni::audio::AudioEngine>();

  title = "Universe Tech Test";
  paused = false;
  settings.overlay = true;

  m_ID = GetTickCount();

  std::cout << "Created engine instance " << m_ID << std::endl;
}

std::shared_ptr<UniEngine> UniEngine::GetInstance() {
  static std::shared_ptr<UniEngine> instance(new UniEngine);
  return instance;
}

void UniEngine::Delete() {
  std::cout << "UniEngine shared pointer has " << GetInstance().use_count()
            << "uses still left in shutdown." << std::endl;
  GetInstance().reset();
}

// Enable physical device features required for this example
void UniEngine::getEnabledFeatures() {
  // Enable sample rate shading filtering if supported
  if (deviceFeatures.sampleRateShading) {
    enabledFeatures.sampleRateShading = VK_TRUE;
  }
  // Enable anisotropic filtering if supported
  if (deviceFeatures.samplerAnisotropy) {
    enabledFeatures.samplerAnisotropy = VK_TRUE;
  }
  // Enable texture compression
  if (deviceFeatures.textureCompressionBC) {
    enabledFeatures.textureCompressionBC = VK_TRUE;
  } else if (deviceFeatures.textureCompressionASTC_LDR) {
    enabledFeatures.textureCompressionASTC_LDR = VK_TRUE;
  } else if (deviceFeatures.textureCompressionETC2) {
    enabledFeatures.textureCompressionETC2 = VK_TRUE;
  }

  // Fill mode non solid is required for wireframe display
  if (deviceFeatures.fillModeNonSolid) {
    enabledFeatures.fillModeNonSolid = VK_TRUE;
    // Wide lines must be present for line width > 1.0f
    if (deviceFeatures.wideLines) {
      enabledFeatures.wideLines = VK_TRUE;
    }
  }

  if (deviceFeatures.tessellationShader) {
    enabledFeatures.tessellationShader = VK_TRUE;
  } else {
    vks::tools::exitFatal("Selected GPU does not support tessellation shaders!",
                          VK_ERROR_FEATURE_NOT_PRESENT);
  }

  // if (deviceFeatures.shaderSampledImageArrayDynamicIndexing) {
  //  enabledFeatures.shaderSampledImageArrayDynamicIndexing = VK_TRUE;
  //} else {
  //  vks::tools::exitFatal("Selected GPU does not support dynamic texture
  //  indexing!",
  //                        VK_ERROR_FEATURE_NOT_PRESENT);
  //}
}

size_t UniEngine::getDynamicAlignment() {
  size_t minUboAlignment =
      vulkanDevice->properties.limits.minUniformBufferOffsetAlignment;

  auto dynamicAlignment = sizeof(glm::mat4);
  if (minUboAlignment > 0) {
    dynamicAlignment =
        (dynamicAlignment + minUboAlignment - 1) & ~(minUboAlignment - 1);
  }

  return dynamicAlignment;
}

void UniEngine::prepare() {
  std::cout << "Initialize engine..." << std::endl;

  std::cout << "Initialize scenegraph..." << std::endl;
  GetSceneManager()->Initialise();

  std::cout << "Initialize input manager..." << std::endl;
  SetupInput();

  std::cout << "Initialize base engine..." << std::endl;
  VulkanExampleBase::prepare();

  std::cout << "Create an audio manager..." << std::endl;
  GetAudioManager()->Init();

  std::cout << "Loading asset registry..." << std::endl;
  GetAssetManager()->LoadRegistry();

  std::cout << "Import all assets ..." << std::endl;
  GetAssetManager()->ImportAll();

  std::cout << "Load level data..." << std::endl;
  GetSceneManager()->LoadScene("testlevel2");
  GetSceneManager()->ActivateScene("testlevel2");

  SetupOverlay();

  prepared = true;
  std::cout << "Initialization complete." << std::endl;
}


void UniEngine::SetupInput() {
  using Input = uni::input::Input;

  m_InputManager = std::make_shared<Input>();
  m_InputManager->Initialize();

  m_InputManager->OnPress(Input::ButtonQuit,
                          [this]() { m_QuitMessageReceived = true; });
  m_InputManager->OnRelease(Input::ButtonPause,
                            [this]() { paused = !paused; });

  m_InputManager->OnRelease(Input::ButtonExperiment, [this]() {
    GetSceneManager()->EmitEvent<InputEvent>({Input::ButtonExperiment, 1.0f});
  });

  m_InputManager->OnRelease(Input::ButtonBoostUp, [this]() {
    GetSceneManager()->EmitEvent<InputEvent>({Input::ButtonBoostUp, 1.0f});
  });
  m_InputManager->OnRelease(Input::ButtonBoostDown, [this]() {
    GetSceneManager()->EmitEvent<InputEvent>({Input::ButtonBoostDown, 1.0f});
  });

  m_InputManager->OnPress(Input::ButtonRollLeft, [this]() {
    GetSceneManager()->EmitEvent<InputEvent>({Input::ButtonRollLeft, 1.0f});
  });
  m_InputManager->OnRelease(Input::ButtonRollLeft, [this]() {
    GetSceneManager()->EmitEvent<InputEvent>({Input::ButtonRollLeft, 0.0f});
  });
  m_InputManager->OnPress(Input::ButtonRollRight, [this]() {
    GetSceneManager()->EmitEvent<InputEvent>({Input::ButtonRollRight, 1.0f});
  });
  m_InputManager->OnRelease(Input::ButtonRollRight, [this]() {
    GetSceneManager()->EmitEvent<InputEvent>({Input::ButtonRollRight, 0.0f});
  });

  m_InputManager->RegisterFloatCallback(
      Input::AxisYaw, [this](float oldValue, float newValue) {
        GetSceneManager()->EmitEvent<InputEvent>({Input::AxisYaw, newValue});
      });
  m_InputManager->RegisterFloatCallback(
      Input::AxisPitch, [this](float oldValue, float newValue) {
        GetSceneManager()->EmitEvent<InputEvent>({Input::AxisPitch, newValue});
      });
  m_InputManager->RegisterFloatCallback(
      Input::AxisThrust, [this](float oldValue, float newValue) {
        GetSceneManager()->EmitEvent<InputEvent>({Input::AxisThrust, newValue});
      });
  m_InputManager->RegisterFloatCallback(
      Input::AxisReverse, [this](float oldValue, float newValue) {
        GetSceneManager()->EmitEvent<InputEvent>(
            {Input::AxisThrust, -newValue});
      });
  m_InputManager->RegisterFloatCallback(
      Input::AxisStrafe, [this](float oldValue, float newValue) {
        GetSceneManager()->EmitEvent<InputEvent>(
            {Input::AxisStrafe, -newValue});
      });
  m_InputManager->RegisterFloatCallback(
      Input::AxisAscend, [this](float oldValue, float newValue) {
        GetSceneManager()->EmitEvent<InputEvent>(
            {Input::AxisAscend, -newValue});
      });
  m_InputManager->RegisterBoolCallback(
      Input::ButtonRightClick, [this](bool oldValue, bool newValue) {
        GetSceneManager()->EmitEvent<InputEvent>(
            {Input::ButtonRightClick, newValue ? 1.f : 0.f});
      });
}

void UniEngine::SetupOverlay()
{
  settings.overlay = settings.overlay && (!benchmark.active);
  if (settings.overlay) {
    vks::UIOverlayCreateInfo overlayCreateInfo = {};
    // Setup default overlay creation info
    overlayCreateInfo.device = vulkanDevice;
    overlayCreateInfo.copyQueue = queue;
    overlayCreateInfo.framebuffers = frameBuffers;
    overlayCreateInfo.colorformat = swapChain.colorFormat;
    overlayCreateInfo.depthformat = depthFormat;
    overlayCreateInfo.width = width;
    overlayCreateInfo.height = height;
    // Virtual function call for example to customize overlay creation
    OnSetupUIOverlay(overlayCreateInfo);
    // Load default shaders if not specified by example
    if (overlayCreateInfo.shaders.empty()) {
      overlayCreateInfo.shaders = {
          loadShader(getAssetPath() + "shaders/base/uioverlay.vert.spv",
                     VK_SHADER_STAGE_VERTEX_BIT),
          loadShader(getAssetPath() + "shaders/base/uioverlay.frag.spv",
                     VK_SHADER_STAGE_FRAGMENT_BIT),
      };
    }
    UIOverlay = new vks::UIOverlay(overlayCreateInfo);
    updateOverlay();

  }
}


void UniEngine::draw() {
  VulkanExampleBase::prepareFrame();

  // Scene rendering
  // Submit work

  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &drawCmdBuffers[currentBuffer];

  VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));

  VulkanExampleBase::submitFrame();
}

void UniEngine::render() {
  // buildCommandBuffers();
  if (!prepared)
    return;

  m_InputManager->Tick();

  draw();

  GetSceneRenderer()->Render();

  if (!paused) {
    GetSceneManager()->Tick(frameTimer);
  }

  if (GetSceneManager()->CheckNewScene()) {
    m_InputManager.reset();
    SetupInput();
  }
}

void UniEngine::viewChanged() {
  GetSceneRenderer()->ViewChanged();
}

void UniEngine::windowResized() {
  GetSceneManager()->CurrentCamera()->aspect = (float)width / (float)height;
  GetSceneManager()->CurrentCamera()->CalculateProjection();
}

void UniEngine::OnUpdateUIOverlay(vks::UIOverlay* overlay) {
  if (overlay->header("Settings")) {
    /*if (overlay->checkBox("wireframe", &m_useWireframe)) {
      ToggleWireframe();
    }*/
    if (overlay->checkBox("Pause camera position", &m_CamPaused)) {
      GetSceneManager()->EmitEvent<CameraPauseEvent>({m_CamPaused});
    }

    GetSceneManager()
        ->CurrentScene()
        ->m_World
        ->each<PlayerControlComponent, TransformComponent, PhysicsComponent>(
            [&](ECS::Entity* ent,
                ECS::ComponentHandle<PlayerControlComponent> player,
                ECS::ComponentHandle<TransformComponent> transform,
                ECS::ComponentHandle<PhysicsComponent> physics) {
              overlay->text("Boost: %.3f", 1.0f);
              overlay->text("Velocity: %.3f m/s",
                            glm::length(physics->m_Velocity));
            });
  }
}

void UniEngine::handleWMMessages(MSG& msg) {
  m_InputManager->HandleWM(msg);
}

void UniEngine::updateOverlay() {
  auto pos = m_InputManager->GetPointerXY();

  // std::cout << "Pos: " << pos.X << ", " << pos.Y << std::endl;

  if (!settings.overlay)
    return;

  ImGuiIO& io = ImGui::GetIO();

  io.DisplaySize = ImVec2((float)width, (float)height);
  io.DeltaTime = frameTimer;

  io.MousePos = ImVec2(pos.X, pos.Y);
  io.MouseDown[0] = m_InputManager->GetButtonState(uni::input::Input::ButtonClick);
  io.MouseDown[1] = m_InputManager->GetButtonState(uni::input::Input::ButtonRightClick);

  ImGui::NewFrame();

  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
  ImGui::SetNextWindowPos(ImVec2(10, 10));
  ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
  ImGui::Begin("Universe Tech", nullptr,
               ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize |
                   ImGuiWindowFlags_NoMove);
  ImGui::TextUnformatted(title.c_str());
  ImGui::TextUnformatted(deviceProperties.deviceName);
  ImGui::Text("%.2f ms/frame (%.1d fps)", (1000.0f / lastFPS), lastFPS);

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,
                      ImVec2(0.0f, 5.0f * UIOverlay->scale));
#endif
  ImGui::PushItemWidth(110.0f * UIOverlay->scale);
  OnUpdateUIOverlay(UIOverlay);
  ImGui::PopItemWidth();
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
  ImGui::PopStyleVar();
#endif

  ImGui::End();

  //  ImGui::SetNextWindowPos(ImVec2(width - 170.f, 10.f));
  //  ImGui::SetNextWindowSize(ImVec2(0.f, 0.f), ImGuiSetCond_FirstUseEver);
  //  ImGui::Begin("Current position:", nullptr,
  //               ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize
  //               |
  //                   ImGuiWindowFlags_NoMove);
  //
  //#if defined(VK_USE_PLATFORM_ANDROID_KHR)
  //  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,
  //                      ImVec2(0.0f, 5.0f * UIOverlay->scale));
  //#endif
  //  /*ImGui::PushItemWidth(110.0f * UIOverlay->scale);
  //  OnUpdateUserUIOverlay(UIOverlay);
  //  ImGui::PopItemWidth();*/
  //#if defined(VK_USE_PLATFORM_ANDROID_KHR)
  //  ImGui::PopStyleVar();
  //#endif
  //
  //  ImGui::End();

  ImGui::PopStyleVar();
  ImGui::Render();

  UIOverlay->update();

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
  if (mouseButtons.left) {
    mouseButtons.left = false;
  }
#endif
}

void UniEngine::OnUpdateUserUIOverlay(vks::UIOverlay* overlay) {
  for (const auto& so : GetSceneManager()->CurrentScene()->m_SceneObjects) {
    if (so->GetComponent<Planet>()) {
      if (overlay->header(so->GetName().c_str())) {
        auto camPos = GetSceneManager()
                          ->CurrentScene()
                          ->GetCameraObject()
                          ->GetTransform()
                          ->GetPosition();
        auto transform = so->GetComponent<TransformComponent>();
        camPos = transform->TransformWSToLocal(camPos);
        auto altitude = so->GetComponent<Planet>()->GetAltitude(camPos);
        overlay->text("Alt: %.3f km", altitude / 1000.0);
        overlay->text("Dist: %.3f km", glm::length(camPos) / 1000.0);
      }
    }
  }
}

std::shared_ptr<uni::render::SceneRenderer> UniEngine::GetSceneRenderer()
{
  return m_SceneManager->GetSceneRenderer();
}
