
#include "UniEngine.h"
#include <assert.h>
#include <algorithm>
#include "UniInput.h"
#include "UniMaterial.h"
#include "UniModel.h"
#include "UniSceneManager.h"
#include "UniSceneRenderer.h"
#include "components/Components.h"
#include "systems/events.h"
#include "vks/VulkanTools.h"

#define ENABLE_VALIDATION true



void UniEngine::Shutdown() {
  std::cout << "Shutting down..." << std::endl;

  SceneManager()->Shutdown();
  
  // Clean up used Vulkan resources
  // Note : Inherited destructor cleans up resources stored in base class

  SceneRenderer()->ShutDown();

  vkFreeCommandBuffers(device, cmdPool, 1, &m_forwardCommandBuffer);
}

UniEngine::~UniEngine() {
  // Shutdown();
}

UniEngine::UniEngine() : VulkanExampleBase(ENABLE_VALIDATION) {
  m_SceneManager = std::make_shared<UniSceneManager>();
  m_SceneRenderer = std::make_shared<UniSceneRenderer>();

  title = "Universe Tech Test";
  paused = false;
  settings.overlay = true;
}

UniEngine& UniEngine::GetInstance() {
  static UniEngine instance;
  return instance;
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

//// Create a frame buffer attachment
// void UniEngine::createAttachment(VkFormat format, VkImageUsageFlagBits usage,
// FrameBufferAttachment *attachment) {
//
//	VkImageAspectFlags aspectMask = 0;
//	VkImageLayout imageLayout;
//
//	attachment->format = format;
//
//	if(usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) {
//		aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//		imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//	}
//	if(usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
//		aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT |
//VK_IMAGE_ASPECT_STENCIL_BIT; 		imageLayout =
//VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
//	}
//
//	assert(aspectMask > 0);
//
//	VkImageCreateInfo image = vks::initializers::imageCreateInfo();
//	image.imageType = VK_IMAGE_TYPE_2D;
//	image.format = format;
//	image.extent.width = offScreenFrameBuf.width;
//	image.extent.height = offScreenFrameBuf.height;
//	image.extent.depth = 1;
//	image.mipLevels = 1;
//	image.arrayLayers = 1;
//	image.samples = SAMPLE_COUNT;
//	image.tiling = VK_IMAGE_TILING_OPTIMAL;
//	image.usage = usage | VK_IMAGE_USAGE_SAMPLED_BIT;
//
//	VkMemoryAllocateInfo memAlloc = vks::initializers::memoryAllocateInfo();
//	VkMemoryRequirements memReqs;
//
//	VK_CHECK_RESULT(vkCreateImage(device, &image, nullptr,
//&attachment->image)); 	vkGetImageMemoryRequirements(device, attachment->image,
//&memReqs); 	memAlloc.allocationSize = memReqs.size; 	memAlloc.memoryTypeIndex =
//vulkanDevice->getMemoryType(memReqs.memoryTypeBits,
//VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT); 	VK_CHECK_RESULT(vkAllocateMemory(device,
//&memAlloc, nullptr, &attachment->mem));
//	VK_CHECK_RESULT(vkBindImageMemory(device, attachment->image,
//attachment->mem, 0));
//
//	VkImageViewCreateInfo imageView =
//vks::initializers::imageViewCreateInfo(); 	imageView.viewType =
//VK_IMAGE_VIEW_TYPE_2D; 	imageView.format = format; 	imageView.subresourceRange =
//{}; 	imageView.subresourceRange.aspectMask = aspectMask;
//	imageView.subresourceRange.baseMipLevel = 0;
//	imageView.subresourceRange.levelCount = 1;
//	imageView.subresourceRange.baseArrayLayer = 0;
//	imageView.subresourceRange.layerCount = 1;
//	imageView.image = attachment->image;
//	VK_CHECK_RESULT(vkCreateImageView(device, &imageView, nullptr,
//&attachment->view));
//}

void UniEngine::prepare() {
  std::cout << "Initialize engine..." << std::endl;

  std::cout << "Initialize scenegraph..." << std::endl;
  SceneManager()->Initialise();

  std::cout << "Initialize input manager..." << std::endl;
  SetupInput();

  std::cout << "Initialize base engine..." << std::endl;
  VulkanExampleBase::prepare();

  std::cout << "Load level data..." << std::endl;
  SceneManager()->LoadScene("testlevel");

  SceneRenderer()->Initialise();

  prepared = true;
  std::cout << "Initialization complete." << std::endl;
}

void UniEngine::SetupInput() {
  m_InputManager = std::make_shared<UniInput>();
  m_InputManager->Initialize();

  m_InputManager->OnPress(UniInput::ButtonQuit,
                          [this]() { m_QuitMessageReceived = true; });
  m_InputManager->OnRelease(UniInput::ButtonPause,
                            [this]() { paused = !paused; });

  m_InputManager->OnRelease(UniInput::ButtonExperiment, [this]() {
    SceneManager()->EmitEvent<InputEvent>(
        {UniInput::ButtonExperiment, 1.0f});
  });

  m_InputManager->OnRelease(UniInput::ButtonBoostUp, [this]() {
    SceneManager()->EmitEvent<InputEvent>(
        {UniInput::ButtonBoostUp, 1.0f});
  });
  m_InputManager->OnRelease(UniInput::ButtonBoostDown, [this]() {
    SceneManager()->EmitEvent<InputEvent>(
        {UniInput::ButtonBoostDown, 1.0f});
  });

  m_InputManager->OnPress(UniInput::ButtonRollLeft, [this]() {
    SceneManager()->EmitEvent<InputEvent>(
        {UniInput::ButtonRollLeft, 1.0f});
  });
  m_InputManager->OnRelease(UniInput::ButtonRollLeft, [this]() {
    SceneManager()->EmitEvent<InputEvent>(
        {UniInput::ButtonRollLeft, 0.0f});
  });
  m_InputManager->OnPress(UniInput::ButtonRollRight, [this]() {
    SceneManager()->EmitEvent<InputEvent>(
        {UniInput::ButtonRollRight, 1.0f});
  });
  m_InputManager->OnRelease(UniInput::ButtonRollRight, [this]() {
    SceneManager()->EmitEvent<InputEvent>(
        {UniInput::ButtonRollRight, 0.0f});
  });

  m_InputManager->RegisterFloatCallback(UniInput::AxisYaw, [this](
                                                               float oldValue,
                                                               float newValue) {
        SceneManager()->EmitEvent<InputEvent>(
            {UniInput::AxisYaw, newValue});
  });
  m_InputManager->RegisterFloatCallback(UniInput::AxisPitch, [this](
                                                                 float oldValue,
                                                                 float
                                                                     newValue) {
        SceneManager()->EmitEvent<InputEvent>(
            {UniInput::AxisPitch, newValue});
  });
  m_InputManager->RegisterFloatCallback(
      UniInput::AxisThrust, [this](float oldValue, float newValue) {
        SceneManager()->EmitEvent<InputEvent>(
            {UniInput::AxisThrust, newValue});
      });
  m_InputManager->RegisterFloatCallback(
      UniInput::AxisReverse, [this](float oldValue, float newValue) {
        SceneManager()->EmitEvent<InputEvent>(
            {UniInput::AxisThrust, -newValue});
      });
  m_InputManager->RegisterFloatCallback(
      UniInput::AxisStrafe, [this](float oldValue, float newValue) {
        SceneManager()->EmitEvent<InputEvent>(
            {UniInput::AxisStrafe, -newValue});
      });
  m_InputManager->RegisterFloatCallback(
      UniInput::AxisAscend, [this](float oldValue, float newValue) {
        SceneManager()->EmitEvent<InputEvent>(
            {UniInput::AxisAscend, -newValue});
      });
  m_InputManager->RegisterBoolCallback(
      UniInput::ButtonRightClick, [this](bool oldValue, bool newValue) {
        SceneManager()->EmitEvent<InputEvent>(
            {UniInput::ButtonRightClick, newValue ? 1.f : 0.f});
      });
}

//// Prepare a new framebuffer for offscreen rendering
//// The contents of this framebuffer are then
//// blitted to our render target
// void UniEngine::prepareOffscreenFramebuffer() {
//	offScreenFrameBuf.width = this->width;
//	offScreenFrameBuf.height = this->height;
//
//	//offScreenFrameBuf.width = FB_DIM;
//	//offScreenFrameBuf.height = FB_DIM;
//
//	// Color attachments
//
//	// (World space) Positions
//	createAttachment(
//		VK_FORMAT_R16G16B16A16_SFLOAT,
//		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
//		&offScreenFrameBuf.position);
//
//	// (World space) Normals
//	createAttachment(
//		VK_FORMAT_R16G16B16A16_SFLOAT,
//		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
//		&offScreenFrameBuf.normal);
//
//	// Albedo (color)
//	createAttachment(
//		VK_FORMAT_R8G8B8A8_UNORM,
//		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
//		&offScreenFrameBuf.albedo);
//
//	// Depth attachment
//
//	// Find a suitable depth format
//	VkFormat attDepthFormat;
//	VkBool32 validDepthFormat =
//vks::tools::getSupportedDepthFormat(physicalDevice, &attDepthFormat);
//	assert(validDepthFormat);
//
//	createAttachment(
//		attDepthFormat,
//		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
//		&offScreenFrameBuf.depth);
//
//	// Set up separate renderpass with references
//	// to the color and depth attachments
//
//	std::array<VkAttachmentDescription, 4> attachmentDescs = {};
//
//	// Init attachment properties
//	for(uint32_t i = 0; i < 4; ++i) {
//		attachmentDescs[i].samples = SAMPLE_COUNT;
//		attachmentDescs[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
//		attachmentDescs[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
//		attachmentDescs[i].stencilLoadOp =
//VK_ATTACHMENT_LOAD_OP_DONT_CARE; 		attachmentDescs[i].stencilStoreOp =
//VK_ATTACHMENT_STORE_OP_DONT_CARE; 		if(i == 3) {
//			attachmentDescs[i].initialLayout =
//VK_IMAGE_LAYOUT_UNDEFINED; 			attachmentDescs[i].finalLayout =
//VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; 		} else {
//			attachmentDescs[i].initialLayout =
//VK_IMAGE_LAYOUT_UNDEFINED; 			attachmentDescs[i].finalLayout =
//VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//		}
//	}
//
//	// Formats
//	attachmentDescs[0].format = offScreenFrameBuf.position.format;
//	attachmentDescs[1].format = offScreenFrameBuf.normal.format;
//	attachmentDescs[2].format = offScreenFrameBuf.albedo.format;
//	attachmentDescs[3].format = offScreenFrameBuf.depth.format;
//
//	std::vector<VkAttachmentReference> colorReferences;
//	colorReferences.push_back({ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
//}); 	colorReferences.push_back({ 1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
//}); 	colorReferences.push_back({ 2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
//});
//
//	VkAttachmentReference depthReference = {};
//	depthReference.attachment = 3;
//	depthReference.layout =
//VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
//
//	VkSubpassDescription subpass = {};
//	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
//	subpass.pColorAttachments = colorReferences.data();
//	subpass.colorAttachmentCount =
//static_cast<uint32_t>(colorReferences.size()); 	subpass.pDepthStencilAttachment
//= &depthReference;
//
//	// Use subpass dependencies for attachment layput transitions
//	std::array<VkSubpassDependency, 2> dependencies;
//
//	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
//	dependencies[0].dstSubpass = 0;
//	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
//	dependencies[0].dstStageMask =
//VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; 	dependencies[0].srcAccessMask =
//VK_ACCESS_MEMORY_READ_BIT; 	dependencies[0].dstAccessMask =
//VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
//	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
//
//	dependencies[1].srcSubpass = 0;
//	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
//	dependencies[1].srcStageMask =
//VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; 	dependencies[1].dstStageMask =
//VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT; 	dependencies[1].srcAccessMask =
//VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
//	dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
//	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
//
//	VkRenderPassCreateInfo renderPassInfo = {};
//	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
//	renderPassInfo.pAttachments = attachmentDescs.data();
//	renderPassInfo.attachmentCount =
//static_cast<uint32_t>(attachmentDescs.size()); 	renderPassInfo.subpassCount =
//1; 	renderPassInfo.pSubpasses = &subpass; 	renderPassInfo.dependencyCount = 2;
//	renderPassInfo.pDependencies = dependencies.data();
//
//	VK_CHECK_RESULT(vkCreateRenderPass(device, &renderPassInfo, nullptr,
//&offScreenFrameBuf.renderPass));
//
//	std::array<VkImageView, 4> attachments;
//	attachments[0] = offScreenFrameBuf.position.view;
//	attachments[1] = offScreenFrameBuf.normal.view;
//	attachments[2] = offScreenFrameBuf.albedo.view;
//	attachments[3] = offScreenFrameBuf.depth.view;
//
//	VkFramebufferCreateInfo fbufCreateInfo = {};
//	fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
//	fbufCreateInfo.pNext = nullptr;
//	fbufCreateInfo.renderPass = offScreenFrameBuf.renderPass;
//	fbufCreateInfo.pAttachments = attachments.data();
//	fbufCreateInfo.attachmentCount =
//static_cast<uint32_t>(attachments.size()); 	fbufCreateInfo.width =
//offScreenFrameBuf.width; 	fbufCreateInfo.height = offScreenFrameBuf.height;
//	fbufCreateInfo.layers = 1;
//	VK_CHECK_RESULT(vkCreateFramebuffer(device, &fbufCreateInfo, nullptr,
//&offScreenFrameBuf.frameBuffer));
//
//	// Create sampler to sample from the color attachments
//	VkSamplerCreateInfo sampler = vks::initializers::samplerCreateInfo();
//	sampler.magFilter = VK_FILTER_NEAREST;
//	sampler.minFilter = VK_FILTER_NEAREST;
//	sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
//	sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
//	sampler.addressModeV = sampler.addressModeU;
//	sampler.addressModeW = sampler.addressModeU;
//	sampler.mipLodBias = 0.0f;
//	sampler.maxAnisotropy = 1.0f;
//	sampler.minLod = 0.0f;
//	sampler.maxLod = 1.0f;
//	sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
//	VK_CHECK_RESULT(vkCreateSampler(device, &sampler, nullptr,
//&m_colorSampler));
//}







void UniEngine::draw() {
  VulkanExampleBase::prepareFrame();

  SceneRenderer()->BuildCommandBuffers();

  // Scene rendering
  // Submit work
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

  SceneRenderer()->Render();

  if (!paused) {
    SceneManager()->Tick(frameTimer);
  }

  if(SceneManager()->CheckNewScene()){
    m_InputManager.reset();
    SetupInput();
  }


}

void UniEngine::viewChanged() {
  SceneRenderer()->ViewChanged();
}

void UniEngine::windowResized() {
  SceneManager()->CurrentCamera()->aspect =
      (float)width / (float)height;
  SceneManager()->CurrentCamera()->CalculateProjection();
}

void UniEngine::OnUpdateUIOverlay(vks::UIOverlay* overlay) {
  if (overlay->header("Settings")) {
    /*if (overlay->checkBox("wireframe", &m_useWireframe)) {
      ToggleWireframe();
    }*/
    if (overlay->checkBox("Pause camera position", &m_CamPaused)) {
      SceneManager()->EmitEvent<CameraPauseEvent>(
          {m_CamPaused});
    }

    SceneManager()
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
  io.MouseDown[0] = m_InputManager->GetButtonState(UniInput::ButtonClick);
  io.MouseDown[1] = m_InputManager->GetButtonState(UniInput::ButtonRightClick);

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
//               ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize |
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
  for (const auto& so : SceneManager()->CurrentScene()->m_SceneObjects) {
    if (so->GetComponent<UniPlanet>()) {
      if (overlay->header(so->GetName().c_str())) {
        auto camPos = SceneManager()
                          ->CurrentScene()
                          ->GetCameraObject()
                          ->GetTransform()
                          ->GetPosition();
        auto transform = so->GetComponent<TransformComponent>();
        camPos = transform->TransformWSToLocal(camPos);
        auto altitude = so->GetComponent<UniPlanet>()->GetAltitude(camPos);
        overlay->text("Alt: %.3f km", altitude / 1000.0);
        overlay->text("Dist: %.3f km", glm::length(camPos) / 1000.0);
      }
    }
  }
}

