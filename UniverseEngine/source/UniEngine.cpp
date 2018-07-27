/*
* Vulkan Example - Multi sampling with explicit resolve for deferred shading example
*
* Copyright (C) 2016 by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#include "UniEngine.h"
#include "vks/VulkanTools.h"
#include <assert.h>
#include <algorithm>
#include "systems/events.h"
#include "components/LightComponent.h"
#include "components/UniPlanet.h"
#include "UniModel.h"
#include "UniScene.h"
#include "UniInput.h"
#include "UniMaterial.h"
#include "components/PlayerMovement.h"

#define ENABLE_VALIDATION true

// Wrapper functions for aligned memory allocation
// There is currently no standard for this in C++ that works across all platforms and vendors, so we abstract this
void* alignedAlloc(size_t size, size_t alignment) {
	void *data = nullptr;
#if defined(_MSC_VER) || defined(__MINGW32__)
	data = _aligned_malloc(size, alignment);
#else 
	int res = posix_memalign(&data, alignment, size);
	if(res != 0)
		data = nullptr;
#endif
	return data;
}

void alignedFree(void* data) {
#if	defined(_MSC_VER) || defined(__MINGW32__)
	_aligned_free(data);
#else 
	free(data);
#endif
}

void UniEngine::Shutdown() {

	std::cout << "Shutting down..." << std::endl;

	// Meshes
	auto models = m_CurrentScene->GetModels();
	for_each(models.begin(), models.end(), [](std::shared_ptr<UniModel> model) {
		model->m_Model.destroy();
		model->m_Texture.destroy();
		model->m_NormalMap.destroy();
	});

	m_CurrentScene.reset();

	// Clean up used Vulkan resources 
	// Note : Inherited destructor cleans up resources stored in base class

	vkDestroySampler(device, m_colorSampler, nullptr);

	// Frame buffer

	// Color attachments
	vkDestroyImageView(device, offScreenFrameBuf.position.view, nullptr);
	vkDestroyImage(device, offScreenFrameBuf.position.image, nullptr);
	vkFreeMemory(device, offScreenFrameBuf.position.mem, nullptr);

	vkDestroyImageView(device, offScreenFrameBuf.normal.view, nullptr);
	vkDestroyImage(device, offScreenFrameBuf.normal.image, nullptr);
	vkFreeMemory(device, offScreenFrameBuf.normal.mem, nullptr);

	vkDestroyImageView(device, offScreenFrameBuf.albedo.view, nullptr);
	vkDestroyImage(device, offScreenFrameBuf.albedo.image, nullptr);
	vkFreeMemory(device, offScreenFrameBuf.albedo.mem, nullptr);

	// Depth attachment
	vkDestroyImageView(device, offScreenFrameBuf.depth.view, nullptr);
	vkDestroyImage(device, offScreenFrameBuf.depth.image, nullptr);
	vkFreeMemory(device, offScreenFrameBuf.depth.mem, nullptr);

	vkDestroyFramebuffer(device, offScreenFrameBuf.frameBuffer, nullptr);

	vkDestroyPipeline(device, pipelines.deferred, nullptr);
	vkDestroyPipeline(device, pipelines.deferredNoMSAA, nullptr);
	vkDestroyPipeline(device, pipelines.offscreen, nullptr);
	vkDestroyPipeline(device, pipelines.offscreenSampleShading, nullptr);
	vkDestroyPipeline(device, pipelines.debug, nullptr);

	if (deviceFeatures.fillModeNonSolid)
	{
	}

	vkDestroyPipelineLayout(device, pipelineLayouts.deferred, nullptr);
	vkDestroyPipelineLayout(device, pipelineLayouts.offscreen, nullptr);


	vkDestroyDescriptorSetLayout(device, m_descriptorSetLayout, nullptr);

	// Uniform buffers
	uniformBuffers.vsOffscreen.destroy();
	uniformBuffers.vsFullScreen.destroy();
	uniformBuffers.modelViews.destroy();
	uniformBuffers.fsLights.destroy();

	vkFreeCommandBuffers(device, cmdPool, 1, &m_offScreenCmdBuffer);


	vkDestroyRenderPass(device, offScreenFrameBuf.renderPass, nullptr);

	vkDestroySemaphore(device, m_offscreenSemaphore, nullptr);
}

UniEngine::~UniEngine() {
	//Shutdown();
}

UniEngine::UniEngine() : VulkanExampleBase(ENABLE_VALIDATION) {

	m_CurrentScene = std::make_shared<UniScene>();

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
	if(deviceFeatures.sampleRateShading) {
		enabledFeatures.sampleRateShading = VK_TRUE;
	}
	// Enable anisotropic filtering if supported
	if(deviceFeatures.samplerAnisotropy) {
		enabledFeatures.samplerAnisotropy = VK_TRUE;
	}
	// Enable texture compression  
	if(deviceFeatures.textureCompressionBC) {
		enabledFeatures.textureCompressionBC = VK_TRUE;
	} else if(deviceFeatures.textureCompressionASTC_LDR) {
		enabledFeatures.textureCompressionASTC_LDR = VK_TRUE;
	} else if(deviceFeatures.textureCompressionETC2) {
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

	if(deviceFeatures.tessellationShader) {
		enabledFeatures.tessellationShader = VK_TRUE;
	} else {
		vks::tools::exitFatal("Selected GPU does not support tessellation shaders!", VK_ERROR_FEATURE_NOT_PRESENT);
	}
}

size_t UniEngine::getDynamicAlignment() {
	size_t minUboAlignment = vulkanDevice->properties.limits.minUniformBufferOffsetAlignment;

	auto dynamicAlignment = sizeof(glm::mat4);
	if(minUboAlignment > 0) {
		dynamicAlignment = (dynamicAlignment + minUboAlignment - 1) & ~(minUboAlignment - 1);
	}

	return dynamicAlignment;
}

// Create a frame buffer attachment
void UniEngine::createAttachment(VkFormat format, VkImageUsageFlagBits usage, FrameBufferAttachment *attachment) {

	VkImageAspectFlags aspectMask = 0;
	VkImageLayout imageLayout;

	attachment->format = format;

	if(usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) {
		aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}
	if(usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
		aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	}

	assert(aspectMask > 0);

	VkImageCreateInfo image = vks::initializers::imageCreateInfo();
	image.imageType = VK_IMAGE_TYPE_2D;
	image.format = format;
	image.extent.width = offScreenFrameBuf.width;
	image.extent.height = offScreenFrameBuf.height;
	image.extent.depth = 1;
	image.mipLevels = 1;
	image.arrayLayers = 1;
	image.samples = SAMPLE_COUNT;
	image.tiling = VK_IMAGE_TILING_OPTIMAL;
	image.usage = usage | VK_IMAGE_USAGE_SAMPLED_BIT;

	VkMemoryAllocateInfo memAlloc = vks::initializers::memoryAllocateInfo();
	VkMemoryRequirements memReqs;

	VK_CHECK_RESULT(vkCreateImage(device, &image, nullptr, &attachment->image));
	vkGetImageMemoryRequirements(device, attachment->image, &memReqs);
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = vulkanDevice->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VK_CHECK_RESULT(vkAllocateMemory(device, &memAlloc, nullptr, &attachment->mem));
	VK_CHECK_RESULT(vkBindImageMemory(device, attachment->image, attachment->mem, 0));

	VkImageViewCreateInfo imageView = vks::initializers::imageViewCreateInfo();
	imageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageView.format = format;
	imageView.subresourceRange = {};
	imageView.subresourceRange.aspectMask = aspectMask;
	imageView.subresourceRange.baseMipLevel = 0;
	imageView.subresourceRange.levelCount = 1;
	imageView.subresourceRange.baseArrayLayer = 0;
	imageView.subresourceRange.layerCount = 1;
	imageView.image = attachment->image;
	VK_CHECK_RESULT(vkCreateImageView(device, &imageView, nullptr, &attachment->view));
}

void UniEngine::prepare() {

	std::cout << "Initialize engine..." << std::endl;
	
	std::cout << "Initialize scenegraph..." << std::endl;
	m_CurrentScene->Initialize(this);
	
	std::cout << "Initialize input manager..." << std::endl;
	SetupInput();
	
	std::cout << "Initialize base engine..." << std::endl;
	VulkanExampleBase::prepare();
	
	std::cout << "initialize scene assets..." << std::endl;
	loadAssets();
	
	std::cout << "Initialize vertex descriptions..." << std::endl;
	setupVertexDescriptions();
	
	std::cout << "Initialize offscreen framebuffer..." << std::endl;
	prepareOffscreenFramebuffer();
	
	std::cout << "Initialize uniform buffers..." << std::endl;
	prepareUniformBuffers();
	
	std::cout << "Initialize descriptor set layouts..." << std::endl;
	setupDescriptorSetLayout();
	
	std::cout << "Initialize pipelines..." << std::endl;
	preparePipelines();
	
	std::cout << "Initialize descriptor pool..." << std::endl;
	setupDescriptorPool();
	
	std::cout << "Initialize descriptor sets..." << std::endl;
	setupDescriptorSets();
	
	std::cout << "Initialize command buffers..." << std::endl;
	buildCommandBuffers();
	
	std::cout << "Initialize deferred command buffers..." << std::endl;
	buildDeferredCommandBuffer();
	
	prepared = true;
	std::cout << "Initialization complete." << std::endl;
}

void UniEngine::SetupInput() {
	
	m_InputManager = std::make_shared<UniInput>();
	m_InputManager->Initialize(height, width);

	m_InputManager->OnPress(UniInput::ButtonQuit, [this](){ m_QuitMessageReceived = true; });
	m_InputManager->OnRelease(UniInput::ButtonPause, [this]() { paused = !paused; });
	m_InputManager->OnRelease(UniInput::ButtonBoostUp, [this]() { m_CurrentScene->m_World->emit<InputEvent>({ UniInput::ButtonBoostUp, 1.0f }); });
	m_InputManager->OnRelease(UniInput::ButtonBoostDown, [this]() { m_CurrentScene->m_World->emit<InputEvent>({ UniInput::ButtonBoostDown, 1.0f }); });

	m_InputManager->OnPress(UniInput::ButtonRollLeft, [this]() { m_CurrentScene->m_World->emit<InputEvent>({ UniInput::ButtonRollLeft, 1.0f }); });
	m_InputManager->OnRelease(UniInput::ButtonRollLeft, [this]() { m_CurrentScene->m_World->emit<InputEvent>({ UniInput::ButtonRollLeft, 0.0f }); });
	m_InputManager->OnPress(UniInput::ButtonRollRight, [this]() { m_CurrentScene->m_World->emit<InputEvent>({ UniInput::ButtonRollRight, 1.0f }); });
	m_InputManager->OnRelease(UniInput::ButtonRollRight, [this]() { m_CurrentScene->m_World->emit<InputEvent>({ UniInput::ButtonRollRight, 0.0f }); });

	m_InputManager->RegisterFloatCallback(UniInput::AxisYaw, [this](float oldValue, float newValue) { m_CurrentScene->m_World->emit<InputEvent>({ UniInput::AxisYaw, newValue }); });
	m_InputManager->RegisterFloatCallback(UniInput::AxisPitch, [this](float oldValue, float newValue) { m_CurrentScene->m_World->emit<InputEvent>({ UniInput::AxisPitch, newValue }); });
	m_InputManager->RegisterFloatCallback(UniInput::AxisThrust, [this](float oldValue, float newValue) { m_CurrentScene->m_World->emit<InputEvent>({ UniInput::AxisThrust, newValue }); });
	m_InputManager->RegisterFloatCallback(UniInput::AxisReverse, [this](float oldValue, float newValue) { m_CurrentScene->m_World->emit<InputEvent>({ UniInput::AxisThrust, -newValue }); });
	m_InputManager->RegisterFloatCallback(UniInput::AxisStrafe, [this](float oldValue, float newValue) { m_CurrentScene->m_World->emit<InputEvent>({ UniInput::AxisStrafe, -newValue }); });
	m_InputManager->RegisterFloatCallback(UniInput::AxisAscend, [this](float oldValue, float newValue) { m_CurrentScene->m_World->emit<InputEvent>({ UniInput::AxisAscend, -newValue }); });
	m_InputManager->RegisterBoolCallback(UniInput::ButtonRightClick, [this](bool oldValue, bool newValue) { m_CurrentScene->m_World->emit<InputEvent>({ UniInput::ButtonRightClick, newValue ? 1.f : 0.f }); });


	
}

void UniEngine::loadAssets() {

	m_CurrentScene->Load(getAssetPath() + "levels/testlevel.json");

}

void UniEngine::setupVertexDescriptions() {
	// Binding description
	vertices.bindingDescriptions.resize(1);
	vertices.bindingDescriptions[0] =
		vks::initializers::vertexInputBindingDescription(
			VERTEX_BUFFER_BIND_ID,
			vertexLayout.stride(),
			VK_VERTEX_INPUT_RATE_VERTEX);

	// Attribute descriptions
	vertices.attributeDescriptions.resize(5);
	// Location 0: Position
	vertices.attributeDescriptions[0] =
		vks::initializers::vertexInputAttributeDescription(
			VERTEX_BUFFER_BIND_ID,
			0,
			VK_FORMAT_R32G32B32_SFLOAT,
			0);
	// Location 1: Texture coordinates
	vertices.attributeDescriptions[1] =
		vks::initializers::vertexInputAttributeDescription(
			VERTEX_BUFFER_BIND_ID,
			1,
			VK_FORMAT_R32G32_SFLOAT,
			sizeof(float) * 3);
	// Location 2: Color
	vertices.attributeDescriptions[2] =
		vks::initializers::vertexInputAttributeDescription(
			VERTEX_BUFFER_BIND_ID,
			2,
			VK_FORMAT_R32G32B32_SFLOAT,
			sizeof(float) * 5);
	// Location 3: Normal
	vertices.attributeDescriptions[3] =
		vks::initializers::vertexInputAttributeDescription(
			VERTEX_BUFFER_BIND_ID,
			3,
			VK_FORMAT_R32G32B32_SFLOAT,
			sizeof(float) * 8);
	// Location 4: Tangent
	vertices.attributeDescriptions[4] =
		vks::initializers::vertexInputAttributeDescription(
			VERTEX_BUFFER_BIND_ID,
			4,
			VK_FORMAT_R32G32B32_SFLOAT,
			sizeof(float) * 11);

	vertices.inputState = vks::initializers::pipelineVertexInputStateCreateInfo();
	vertices.inputState.vertexBindingDescriptionCount = static_cast<uint32_t>(vertices.bindingDescriptions.size());
	vertices.inputState.pVertexBindingDescriptions = vertices.bindingDescriptions.data();
	vertices.inputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertices.attributeDescriptions.size());
	vertices.inputState.pVertexAttributeDescriptions = vertices.attributeDescriptions.data();
}

// Prepare a new framebuffer for offscreen rendering
// The contents of this framebuffer are then
// blitted to our render target
void UniEngine::prepareOffscreenFramebuffer() {
	offScreenFrameBuf.width = this->width;
	offScreenFrameBuf.height = this->height;

	//offScreenFrameBuf.width = FB_DIM;
	//offScreenFrameBuf.height = FB_DIM;

	// Color attachments

	// (World space) Positions
	createAttachment(
		VK_FORMAT_R16G16B16A16_SFLOAT,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		&offScreenFrameBuf.position);

	// (World space) Normals
	createAttachment(
		VK_FORMAT_R16G16B16A16_SFLOAT,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		&offScreenFrameBuf.normal);

	// Albedo (color)
	createAttachment(
		VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		&offScreenFrameBuf.albedo);

	// Depth attachment

	// Find a suitable depth format
	VkFormat attDepthFormat;
	VkBool32 validDepthFormat = vks::tools::getSupportedDepthFormat(physicalDevice, &attDepthFormat);
	assert(validDepthFormat);

	createAttachment(
		attDepthFormat,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		&offScreenFrameBuf.depth);

	// Set up separate renderpass with references
	// to the color and depth attachments

	std::array<VkAttachmentDescription, 4> attachmentDescs = {};

	// Init attachment properties
	for(uint32_t i = 0; i < 4; ++i) {
		attachmentDescs[i].samples = SAMPLE_COUNT;
		attachmentDescs[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachmentDescs[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachmentDescs[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDescs[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		if(i == 3) {
			attachmentDescs[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachmentDescs[i].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		} else {
			attachmentDescs[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachmentDescs[i].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}
	}

	// Formats
	attachmentDescs[0].format = offScreenFrameBuf.position.format;
	attachmentDescs[1].format = offScreenFrameBuf.normal.format;
	attachmentDescs[2].format = offScreenFrameBuf.albedo.format;
	attachmentDescs[3].format = offScreenFrameBuf.depth.format;

	std::vector<VkAttachmentReference> colorReferences;
	colorReferences.push_back({ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
	colorReferences.push_back({ 1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
	colorReferences.push_back({ 2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });

	VkAttachmentReference depthReference = {};
	depthReference.attachment = 3;
	depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.pColorAttachments = colorReferences.data();
	subpass.colorAttachmentCount = static_cast<uint32_t>(colorReferences.size());
	subpass.pDepthStencilAttachment = &depthReference;

	// Use subpass dependencies for attachment layput transitions
	std::array<VkSubpassDependency, 2> dependencies;

	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.pAttachments = attachmentDescs.data();
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescs.size());
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 2;
	renderPassInfo.pDependencies = dependencies.data();

	VK_CHECK_RESULT(vkCreateRenderPass(device, &renderPassInfo, nullptr, &offScreenFrameBuf.renderPass));

	std::array<VkImageView, 4> attachments;
	attachments[0] = offScreenFrameBuf.position.view;
	attachments[1] = offScreenFrameBuf.normal.view;
	attachments[2] = offScreenFrameBuf.albedo.view;
	attachments[3] = offScreenFrameBuf.depth.view;

	VkFramebufferCreateInfo fbufCreateInfo = {};
	fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fbufCreateInfo.pNext = nullptr;
	fbufCreateInfo.renderPass = offScreenFrameBuf.renderPass;
	fbufCreateInfo.pAttachments = attachments.data();
	fbufCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	fbufCreateInfo.width = offScreenFrameBuf.width;
	fbufCreateInfo.height = offScreenFrameBuf.height;
	fbufCreateInfo.layers = 1;
	VK_CHECK_RESULT(vkCreateFramebuffer(device, &fbufCreateInfo, nullptr, &offScreenFrameBuf.frameBuffer));

	// Create sampler to sample from the color attachments
	VkSamplerCreateInfo sampler = vks::initializers::samplerCreateInfo();
	sampler.magFilter = VK_FILTER_NEAREST;
	sampler.minFilter = VK_FILTER_NEAREST;
	sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sampler.addressModeV = sampler.addressModeU;
	sampler.addressModeW = sampler.addressModeU;
	sampler.mipLodBias = 0.0f;
	sampler.maxAnisotropy = 1.0f;
	sampler.minLod = 0.0f;
	sampler.maxLod = 1.0f;
	sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	VK_CHECK_RESULT(vkCreateSampler(device, &sampler, nullptr, &m_colorSampler));
}

// Prepare and initialize uniform buffer containing shader uniforms
void UniEngine::prepareUniformBuffers() {
	// Fullscreen vertex shader
	VK_CHECK_RESULT(vulkanDevice->createBuffer(
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&uniformBuffers.vsFullScreen,
		sizeof(uboVS)));

	// Deferred vertex shader
	VK_CHECK_RESULT(vulkanDevice->createBuffer(
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&uniformBuffers.vsOffscreen,
		sizeof(uboOffscreenVS)));

	auto models = m_CurrentScene->GetModels();
	auto dynamicAlignment = getDynamicAlignment();
	size_t bufferSize = std::max(static_cast<int>(models.size()), 1) * dynamicAlignment;
	uboModelMatDynamic.model = (glm::mat4 *)alignedAlloc(bufferSize, dynamicAlignment);
	assert(uboModelMatDynamic.model);

	// Deferred vertex shader dynamic
	VK_CHECK_RESULT(vulkanDevice->createBuffer(
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, 
		&uniformBuffers.modelViews, bufferSize));


	// Deferred fragment shader
	VK_CHECK_RESULT(vulkanDevice->createBuffer(
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&uniformBuffers.fsLights,
		sizeof(uboFragmentLights)));

	// Map persistent
	VK_CHECK_RESULT(uniformBuffers.vsFullScreen.map());
	VK_CHECK_RESULT(uniformBuffers.vsOffscreen.map());
	VK_CHECK_RESULT(uniformBuffers.modelViews.map());
	VK_CHECK_RESULT(uniformBuffers.fsLights.map());

	// Update
	updateUniformBuffersScreen();
	updateUniformBufferDeferredMatrices();
	updateUniformBufferDeferredLights();
	updateDynamicUniformBuffers();
}

void UniEngine::setupDescriptorSetLayout() {
	// Deferred shading layout
	std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings =
	{
		// Binding 0 : Vertex shader uniform buffer
		vks::initializers::descriptorSetLayoutBinding(
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			VK_SHADER_STAGE_VERTEX_BIT,
			0),
		// Binding 1 : Position texture target / Scene colormap
		vks::initializers::descriptorSetLayoutBinding(
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			1),
		// Binding 2 : Normals texture target
		vks::initializers::descriptorSetLayoutBinding(
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			2),
		// Binding 3 : Albedo texture target
		vks::initializers::descriptorSetLayoutBinding(
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			3),
		// Binding 4 : Fragment shader uniform buffer
		vks::initializers::descriptorSetLayoutBinding(
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			4),
		// Binding 5 : Vertex shader uniform buffer dynamic
		vks::initializers::descriptorSetLayoutBinding(
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
			VK_SHADER_STAGE_VERTEX_BIT,
			5),

	};

	VkDescriptorSetLayoutCreateInfo descriptorLayout =
		vks::initializers::descriptorSetLayoutCreateInfo(
			setLayoutBindings.data(),
			static_cast<uint32_t>(setLayoutBindings.size()));

	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorLayout, nullptr, &m_descriptorSetLayout));


	VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = 
		vks::initializers::pipelineLayoutCreateInfo(
			&m_descriptorSetLayout,
			1);
	
	VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pPipelineLayoutCreateInfo, nullptr, &pipelineLayouts.deferred));

	// Offscreen (scene) rendering pipeline layout
	VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pPipelineLayoutCreateInfo, nullptr, &pipelineLayouts.offscreen));



}

void UniEngine::preparePipelines() {
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState =
		vks::initializers::pipelineInputAssemblyStateCreateInfo(
			VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			0,
			VK_FALSE);

	VkPipelineRasterizationStateCreateInfo rasterizationState =
		vks::initializers::pipelineRasterizationStateCreateInfo(
			VK_POLYGON_MODE_FILL,
			VK_CULL_MODE_NONE, // TODO: debug for backface culling!
			VK_FRONT_FACE_CLOCKWISE,
			0);

	VkPipelineColorBlendAttachmentState blendAttachmentState =
		vks::initializers::pipelineColorBlendAttachmentState(
			0xf,
			VK_FALSE);

	VkPipelineColorBlendStateCreateInfo colorBlendState =
		vks::initializers::pipelineColorBlendStateCreateInfo(
			1,
			&blendAttachmentState);

	VkPipelineDepthStencilStateCreateInfo depthStencilState =
		vks::initializers::pipelineDepthStencilStateCreateInfo(
			VK_TRUE,
			VK_TRUE,
			VK_COMPARE_OP_GREATER_OR_EQUAL);

	VkPipelineViewportStateCreateInfo viewportState =
		vks::initializers::pipelineViewportStateCreateInfo(1, 1, 0);

	VkPipelineMultisampleStateCreateInfo multisampleState =
		vks::initializers::pipelineMultisampleStateCreateInfo(
			VK_SAMPLE_COUNT_1_BIT,
			0);

	std::vector<VkDynamicState> dynamicStateEnables = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};
	VkPipelineDynamicStateCreateInfo dynamicState =
		vks::initializers::pipelineDynamicStateCreateInfo(
			dynamicStateEnables.data(),
			static_cast<uint32_t>(dynamicStateEnables.size()),
			0);

	// Final fullscreen pass pipeline
	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;

	VkGraphicsPipelineCreateInfo pipelineCreateInfo =
		vks::initializers::pipelineCreateInfo(
			pipelineLayouts.deferred,
			renderPass,
			0);

	pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
	pipelineCreateInfo.pRasterizationState = &rasterizationState;
	pipelineCreateInfo.pColorBlendState = &colorBlendState;
	pipelineCreateInfo.pMultisampleState = &multisampleState;
	pipelineCreateInfo.pViewportState = &viewportState;
	pipelineCreateInfo.pDepthStencilState = &depthStencilState;
	pipelineCreateInfo.pDynamicState = &dynamicState;
	pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineCreateInfo.pStages = shaderStages.data();

	// Deferred

	// Empty vertex input state, quads are generated by the vertex shader
	VkPipelineVertexInputStateCreateInfo emptyInputState = vks::initializers::pipelineVertexInputStateCreateInfo();
	pipelineCreateInfo.pVertexInputState = &emptyInputState;
	pipelineCreateInfo.layout = pipelineLayouts.deferred;

	// Use specialization constants to pass number of samples to the shader (used for MSAA resolve)
	VkSpecializationMapEntry specializationEntry{};
	specializationEntry.constantID = 0;
	specializationEntry.offset = 0;
	specializationEntry.size = sizeof(uint32_t);

	uint32_t specializationData = SAMPLE_COUNT;

	VkSpecializationInfo specializationInfo;
	specializationInfo.mapEntryCount = 1;
	specializationInfo.pMapEntries = &specializationEntry;
	specializationInfo.dataSize = sizeof(specializationData);
	specializationInfo.pData = &specializationData;

	// With MSAA
	shaderStages[0] = loadShader(getAssetPath() + "shaders/deferredmultisampling/deferred.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	shaderStages[1] = loadShader(getAssetPath() + "shaders/deferredmultisampling/deferred.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
	shaderStages[1].pSpecializationInfo = &specializationInfo;
	VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCreateInfo, nullptr, &pipelines.deferred));

	// No MSAA (1 sample)
	specializationData = 1;
	VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCreateInfo, nullptr, &pipelines.deferredNoMSAA));

	// Debug display pipeline
	shaderStages[0] = loadShader(getAssetPath() + "shaders/deferredmultisampling/debug.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	shaderStages[1] = loadShader(getAssetPath() + "shaders/deferredmultisampling/debug.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
	VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCreateInfo, nullptr, &pipelines.debug));

	// Offscreen scene rendering pipeline
	pipelineCreateInfo.pVertexInputState = &vertices.inputState;
	shaderStages[0] = loadShader(getAssetPath() + "shaders/deferredmultisampling/mrt.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	shaderStages[1] = loadShader(getAssetPath() + "shaders/deferredmultisampling/mrt.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

	multisampleState.rasterizationSamples = SAMPLE_COUNT;
	multisampleState.alphaToCoverageEnable = VK_TRUE;

	// Separate render pass
	pipelineCreateInfo.renderPass = offScreenFrameBuf.renderPass;

	// Separate layout
	pipelineCreateInfo.layout = pipelineLayouts.offscreen;

	// Blend attachment states required for all color attachments
	// This is important, as color write mask will otherwise be 0x0 and you
	// won't see anything rendered to the attachment
	std::array<VkPipelineColorBlendAttachmentState, 3> blendAttachmentStates = {
		vks::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE),
		vks::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE),
		vks::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE)
	};

	colorBlendState.attachmentCount = static_cast<uint32_t>(blendAttachmentStates.size());
	colorBlendState.pAttachments = blendAttachmentStates.data();

	VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCreateInfo, nullptr, &pipelines.offscreen));

	multisampleState.sampleShadingEnable = VK_TRUE;
	multisampleState.minSampleShading = 0.25f;
	VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCreateInfo, nullptr, &pipelines.offscreenSampleShading));

	for(auto& material : m_MaterialInstances) {
		material->SetupMaterial(pipelineCreateInfo);
	}


}

void UniEngine::setupDescriptorPool() {
	auto modelCount = static_cast<uint32_t>(std::max((int)m_CurrentScene->GetModels().size(), 1)) * 2;

	std::vector<VkDescriptorPoolSize> poolSizes =
	{
		vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3 * modelCount + 1),
		vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 2 * modelCount + 1),
		vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 5 * modelCount + 5 + 5)
	};

	VkDescriptorPoolCreateInfo descriptorPoolInfo =
		vks::initializers::descriptorPoolCreateInfo(
			static_cast<uint32_t>(poolSizes.size()),
			poolSizes.data(),
			static_cast<uint32_t>(modelCount / 2 + 5));

	VK_CHECK_RESULT(vkCreateDescriptorPool(device, &descriptorPoolInfo, nullptr, &descriptorPool));
}

void UniEngine::setupDescriptorSets() {
	std::vector<VkWriteDescriptorSet> writeDescriptorSets;

	// Textured quad descriptor set
	VkDescriptorSetAllocateInfo allocInfo =
		vks::initializers::descriptorSetAllocateInfo(
			descriptorPool,
			&m_descriptorSetLayout,
			1);

	VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, &m_descriptorSet));


	// Image descriptors for the offscreen color attachments
	VkDescriptorImageInfo texDescriptorPosition =
		vks::initializers::descriptorImageInfo(
			m_colorSampler,
			offScreenFrameBuf.position.view,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	VkDescriptorImageInfo texDescriptorNormal =
		vks::initializers::descriptorImageInfo(
			m_colorSampler,
			offScreenFrameBuf.normal.view,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	VkDescriptorImageInfo texDescriptorAlbedo =
		vks::initializers::descriptorImageInfo(
			m_colorSampler,
			offScreenFrameBuf.albedo.view,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	writeDescriptorSets = {
		// Binding 0 : Vertex shader uniform buffer
		vks::initializers::writeDescriptorSet(
			m_descriptorSet,
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			0,
			&uniformBuffers.vsFullScreen.descriptor),
		// Binding 1 : Position texture target
		vks::initializers::writeDescriptorSet(
			m_descriptorSet,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			1,
			&texDescriptorPosition),
		// Binding 2 : Normals texture target
		vks::initializers::writeDescriptorSet(
			m_descriptorSet,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			2,
			&texDescriptorNormal),
		// Binding 3 : Albedo texture target
		vks::initializers::writeDescriptorSet(
			m_descriptorSet,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			3,
			&texDescriptorAlbedo),
		// Binding 4 : Fragment shader uniform buffer
		vks::initializers::writeDescriptorSet(
			m_descriptorSet,
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			4,
			&uniformBuffers.fsLights.descriptor),
	};

	vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);

	// Models
	auto models = m_CurrentScene->GetModels();
	for_each(models.begin(), models.end(), [allocInfo, this](std::shared_ptr<UniModel> model) {

		VkResult res = (vkAllocateDescriptorSets(device, &allocInfo, &model->m_DescriptorSet));
		if(res != VK_SUCCESS)
		{
			std::cout << "Fatal : VkResult is \"" << vks::tools::errorString(res) << "\" in " << __FILE__ << " at line " << __LINE__ << std::endl;
			assert(res == VK_SUCCESS);
		}

		std::vector<VkWriteDescriptorSet> modelWriteDescriptorSets =
		{
			// Binding 0: Vertex shader uniform buffer
			vks::initializers::writeDescriptorSet(
				model->m_DescriptorSet,
				VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				0,
				&uniformBuffers.vsOffscreen.descriptor),
			// Binding 1: Color map
			vks::initializers::writeDescriptorSet(
				model->m_DescriptorSet,
				VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				1,
				&model->m_Texture.descriptor),
			// Binding 2: Normal map
			vks::initializers::writeDescriptorSet(
				model->m_DescriptorSet,
				VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				2,
				&model->m_NormalMap.descriptor),
			// Binding 5 : Vertex shader uniform buffer
			vks::initializers::writeDescriptorSet(
				model->m_DescriptorSet,
				VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
				5,
				&uniformBuffers.modelViews.descriptor),
		};
		vkUpdateDescriptorSets(device, static_cast<uint32_t>(modelWriteDescriptorSets.size()), modelWriteDescriptorSets.data(), 0, nullptr);
	});

}

void UniEngine::buildCommandBuffers() {
	VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();

	VkClearValue clearValues[2];
	clearValues[0].color = { { 1.0f, 1.0f, 1.0f, 0.0f } };
	clearValues[1].depthStencil = { 0.0f, 0 };

	VkRenderPassBeginInfo renderPassBeginInfo = vks::initializers::renderPassBeginInfo();
	renderPassBeginInfo.renderPass = renderPass;
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	renderPassBeginInfo.renderArea.extent.width = width;
	renderPassBeginInfo.renderArea.extent.height = height;
	renderPassBeginInfo.clearValueCount = 2;
	renderPassBeginInfo.pClearValues = clearValues;

	for(int32_t i = 0; i < drawCmdBuffers.size(); ++i) {
		// Set target frame buffer
		renderPassBeginInfo.framebuffer = frameBuffers[i];

		VK_CHECK_RESULT(vkBeginCommandBuffer(drawCmdBuffers[i], &cmdBufInfo));

		vkCmdBeginRenderPass(drawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport = vks::initializers::viewport((float)width, (float)height, 0.0f, 1.0f);
		vkCmdSetViewport(drawCmdBuffers[i], 0, 1, &viewport);

		VkRect2D scissor = vks::initializers::rect2D(width, height, 0, 0);
		vkCmdSetScissor(drawCmdBuffers[i], 0, 1, &scissor);

		uint32_t dummy = 0;
		vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayouts.deferred, 0, 1, &m_descriptorSet, 1, &dummy);

		if(m_debugDisplay) {
			vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.debug);
			vkCmdDraw(drawCmdBuffers[i], 3, 1, 0, 0);
			// Move viewport to display final composition in lower right corner
			viewport.x = viewport.width * 0.5f;
			viewport.y = viewport.height * 0.5f;
			viewport.width = (float)width * 0.5f;
			viewport.height = (float)height * 0.5f;
			vkCmdSetViewport(drawCmdBuffers[i], 0, 1, &viewport);
		}

		GetScene()->GetCameraComponent()->aspect = (float)viewport.width / (float)viewport.height;
		GetScene()->GetCameraComponent()->CalculateProjection();

		// Final composition as full screen quad
		vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_useMSAA ? pipelines.deferred : pipelines.deferredNoMSAA);
		vkCmdDraw(drawCmdBuffers[i], 3, 1, 0, 0);

		vkCmdEndRenderPass(drawCmdBuffers[i]);

		VK_CHECK_RESULT(vkEndCommandBuffer(drawCmdBuffers[i]));
	}
}

// Build command buffer for rendering the scene to the offscreen frame buffer attachments
void UniEngine::buildDeferredCommandBuffer() {
	if(m_offScreenCmdBuffer == VK_NULL_HANDLE) {
		m_offScreenCmdBuffer = VulkanExampleBase::createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, false);
	}

	// Create a semaphore used to synchronize offscreen rendering and usage
	if(m_offscreenSemaphore == VK_NULL_HANDLE) {
		VkSemaphoreCreateInfo semaphoreCreateInfo = vks::initializers::semaphoreCreateInfo();
		VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &m_offscreenSemaphore));
	}

	VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();

	// Clear values for all attachments written in the fragment sahder
	std::array<VkClearValue, 4> clearValues;
	clearValues[0].color = clearValues[1].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
	clearValues[2].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
	clearValues[3].depthStencil = { 0.0f, 0 };

	VkRenderPassBeginInfo renderPassBeginInfo = vks::initializers::renderPassBeginInfo();
	renderPassBeginInfo.renderPass = offScreenFrameBuf.renderPass;
	renderPassBeginInfo.framebuffer = offScreenFrameBuf.frameBuffer;
	renderPassBeginInfo.renderArea.extent.width = offScreenFrameBuf.width;
	renderPassBeginInfo.renderArea.extent.height = offScreenFrameBuf.height;
	renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassBeginInfo.pClearValues = clearValues.data();

	VK_CHECK_RESULT(vkBeginCommandBuffer(m_offScreenCmdBuffer, &cmdBufInfo));

	vkCmdBeginRenderPass(m_offScreenCmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport = vks::initializers::viewport((float)offScreenFrameBuf.width, (float)offScreenFrameBuf.height, 0.0f, 1.0f);
	vkCmdSetViewport(m_offScreenCmdBuffer, 0, 1, &viewport);

	VkRect2D scissor = vks::initializers::rect2D(offScreenFrameBuf.width, offScreenFrameBuf.height, 0, 0);
	vkCmdSetScissor(m_offScreenCmdBuffer, 0, 1, &scissor);

	vkCmdBindPipeline(m_offScreenCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_useSampleShading ? pipelines.offscreenSampleShading : pipelines.offscreen);

	// bind models

	auto dynamicAlignment = getDynamicAlignment();
	int index = 0;

	auto models = m_CurrentScene->GetModels();
	for_each(models.begin(), models.end(), [this, &index, dynamicAlignment](std::shared_ptr<UniModel> model) {
		VkDeviceSize offsets[1] = { 0 };
		uint32_t dynamicOffset = index * static_cast<uint32_t>(dynamicAlignment);
		vkCmdBindDescriptorSets(m_offScreenCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayouts.offscreen, 0, 1, &model->m_DescriptorSet, 1, &dynamicOffset);
		vkCmdBindVertexBuffers(m_offScreenCmdBuffer, VERTEX_BUFFER_BIND_ID, 1, &model->m_Model.vertices.buffer, offsets);
		vkCmdBindIndexBuffer(m_offScreenCmdBuffer, model->m_Model.indices.buffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(m_offScreenCmdBuffer, model->m_Model.indexCount, 1, 0, 0, 0);
		index++;
	});

	for(const auto& material : m_MaterialInstances) {
		material->AddToCommandBuffer(m_offScreenCmdBuffer);
	}


	vkCmdEndRenderPass(m_offScreenCmdBuffer);

	VK_CHECK_RESULT(vkEndCommandBuffer(m_offScreenCmdBuffer));
}


void UniEngine::updateUniformBuffersScreen() {
	if(m_debugDisplay) {
		uboVS.projection = glm::ortho(0.0f, 2.0f, 0.0f, 2.0f, -1.0f, 1.0f);
	} else {
		uboVS.projection = glm::ortho(0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f);
	}
	uboVS.model = glm::mat4(1.0f);

	memcpy(uniformBuffers.vsFullScreen.mapped, &uboVS, sizeof(uboVS));
}

void UniEngine::updateUniformBufferDeferredMatrices() {
	uboOffscreenVS.projection = GetScene()->GetCameraComponent()->matrices.projection;
	uboOffscreenVS.view = GetScene()->GetCameraComponent()->matrices.view;
	uboOffscreenVS.model = glm::mat4(1.f);
	memcpy(uniformBuffers.vsOffscreen.mapped, &uboOffscreenVS, sizeof(uboOffscreenVS));
}

// Update fragment shader light position uniform block
void UniEngine::updateUniformBufferDeferredLights() {
	
	// each scene light into uboFragmentLights.lights
	uint32_t lightCount = 0;
	GetScene()->m_World->each<TransformComponent, LightComponent>([&](ECS::Entity* ent, ECS::ComponentHandle<TransformComponent> transform, ECS::ComponentHandle<LightComponent> light) {
		//std::cout << "Found a light! " << lightCount;
		if(light->enabled && lightCount < 256) {
			auto lPos = glm::vec4(transform->TransformLocalToWS(transform->m_dPos), 0);
			auto lCol = light->color;
			uboFragmentLights.lights[lightCount].color = lCol;
			uboFragmentLights.lights[lightCount].radius = light->radius;
			uboFragmentLights.lights[lightCount].position = lPos;
			lightCount++;
			//std::cout << ", radius: " << light->radius;
			//std::cout << ", pos: " << lPos.x << ", " << lPos.y << ", " << lPos.z << ". ";
			//std::cout << ", col: " << lCol.r << ", " << lCol.g << ", " << lCol.b << ", " << lCol.a << ". " << std::endl;
		} else {
			std::cout << "Light is disabled!" << std::endl;
		}
	});

	uboFragmentLights.viewPos = glm::vec4(GetScene()->GetCameraComponent()->GetPosition(), 0.0f) * glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f);
	uboFragmentLights.numLights = lightCount;

	//std::cout << "Enabled lights: " << lightCount << std::endl;

	memcpy(uniformBuffers.fsLights.mapped, &uboFragmentLights, sizeof(uboFragmentLights));
}

void UniEngine::updateDynamicUniformBuffers() {
	int index = 0;
	auto dynamicAlignment = getDynamicAlignment();
	auto models = m_CurrentScene->GetModels();
	for_each(models.begin(), models.end(), [this, &index, dynamicAlignment](std::shared_ptr<UniModel> model) {
		glm::mat4* modelMat = (glm::mat4*)(((uint64_t)uboModelMatDynamic.model + (index * dynamicAlignment)));
		*modelMat = model->GetTransform()->GetModelMat();
		//std::cout << "Updating model matrix index: " << index << std::endl;
		index++;
	});

	memcpy(uniformBuffers.modelViews.mapped, uboModelMatDynamic.model, uniformBuffers.modelViews.size);
	// Flush to make changes visible to the host 
	VkMappedMemoryRange memoryRange = vks::initializers::mappedMemoryRange();
	memoryRange.memory = uniformBuffers.modelViews.memory;
	memoryRange.size = uniformBuffers.modelViews.size;
	vkFlushMappedMemoryRanges(device, 1, &memoryRange);
}

void UniEngine::draw() {
	VulkanExampleBase::prepareFrame();

	// Offscreen rendering

	// Wait for swap chain presentation to finish
	submitInfo.pWaitSemaphores = &semaphores.presentComplete;
	// Signal ready with offscreen semaphore
	submitInfo.pSignalSemaphores = &m_offscreenSemaphore;

	std::array<VkCommandBuffer, 1> commandBuffers = {
		m_offScreenCmdBuffer,
	};

	// Submit work
	submitInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());
	submitInfo.pCommandBuffers = commandBuffers.data();
	VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));

	// Scene rendering

	// Wait for offscreen semaphore
	submitInfo.pWaitSemaphores = &m_offscreenSemaphore;
	// Signal ready with render complete semaphpre
	submitInfo.pSignalSemaphores = &semaphores.renderComplete;

	// Submit work
	submitInfo.pCommandBuffers = &drawCmdBuffers[currentBuffer];
	VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));

	VulkanExampleBase::submitFrame();
}

void UniEngine::render() {

	m_InputManager->Tick();

	buildDeferredCommandBuffer();
	if(!prepared)
		return;
	draw();
	updateUniformBufferDeferredLights();
	if(!paused)
		m_CurrentScene->Tick(frameTimer);
	updateDynamicUniformBuffers();
}

void UniEngine::viewChanged() {
	updateUniformBufferDeferredMatrices();
}

void UniEngine::windowResized() {
	GetScene()->GetCameraComponent()->aspect = (float)width / (float)height;
	GetScene()->GetCameraComponent()->CalculateProjection();
}


void UniEngine::OnUpdateUIOverlay(vks::UIOverlay *overlay) {
	if(overlay->header("Settings")) {
		if(overlay->checkBox("Display render targets", &m_debugDisplay)) {
			buildCommandBuffers();
			updateUniformBuffersScreen();
		}
		if(overlay->checkBox("MSAA", &m_useMSAA)) {
			buildCommandBuffers();
		}
		if(vulkanDevice->features.sampleRateShading) {
			if(overlay->checkBox("Sample rate shading", &m_useSampleShading)) {
				buildDeferredCommandBuffer();
			}
		}
		
		if(overlay->checkBox("Pause camera position", &m_CamPaused)) {
			GetScene()->m_World->emit<CameraPauseEvent>({ m_CamPaused });
		}

		GetScene()->m_World->each<PlayerControlComponent, MovementComponent>([&](ECS::Entity* ent, ECS::ComponentHandle<PlayerControlComponent> player, ECS::ComponentHandle<MovementComponent> movement) {
			overlay->text("Boost: %.3f", movement->m_BoostFactor);
		});
		
	}
}

void UniEngine::ToggleWireframe()
{
	preparePipelines();
}

void UniEngine::handleWMMessages(MSG& msg) {
	m_InputManager->HandleWM(msg);
}

void UniEngine::updateOverlay() {

	auto pos = m_InputManager->GetPointerXY();

	//std::cout << "Pos: " << pos.X << ", " << pos.Y << std::endl;

	if(!settings.overlay)
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
	ImGui::Begin("Universe Tech", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
	ImGui::TextUnformatted(title.c_str());
	ImGui::TextUnformatted(deviceProperties.deviceName);
	ImGui::Text("%.2f ms/frame (%.1d fps)", (1000.0f / lastFPS), lastFPS);

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 5.0f * UIOverlay->scale));
#endif
	ImGui::PushItemWidth(110.0f * UIOverlay->scale);
	OnUpdateUIOverlay(UIOverlay);
	ImGui::PopItemWidth();
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
	ImGui::PopStyleVar();
#endif

	ImGui::End();


	ImGui::SetNextWindowPos(ImVec2((uint32_t)width - 170, 10));
	ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
	ImGui::Begin("Current position:", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 5.0f * UIOverlay->scale));
#endif
	ImGui::PushItemWidth(110.0f * UIOverlay->scale);
	OnUpdateUserUIOverlay(UIOverlay);
	ImGui::PopItemWidth();
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
	ImGui::PopStyleVar();
#endif

	ImGui::End();


	ImGui::PopStyleVar();
	ImGui::Render();

	UIOverlay->update();

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
	if(mouseButtons.left) {
		mouseButtons.left = false;
	}
#endif
}

void UniEngine::RegisterMaterial(std::shared_ptr<UniMaterial> mat) {
	m_MaterialInstances.push_back(mat);
}

void UniEngine::UnRegisterMaterial(std::shared_ptr<UniMaterial> mat) {
	int i = 0;
	while(i < m_MaterialInstances.size()) {
		if(m_MaterialInstances[i] == mat) {
			m_MaterialInstances.erase(m_MaterialInstances.begin() + i);
			i = 0;
		}
		i++;
	}
}

void UniEngine::OnUpdateUserUIOverlay(vks::UIOverlay *overlay) {
	for(const auto& so : GetScene()->m_SceneObjects) {
		if(so->GetComponent<UniPlanet>()) {
			if(overlay->header(so->GetName().c_str())) {
			auto camPos = GetScene()->GetCameraObject()->GetTransform()->GetPosition();
			auto transform = so->GetComponent<TransformComponent>();
			camPos = transform->TransformWSToLocal(camPos);
			auto altitude = so->GetComponent<UniPlanet>()->GetAltitude(camPos);
			overlay->text("Alt: %.5f km", altitude);
			overlay->text("Dist: %.5f km", glm::length(camPos));
			}
		}
	}
}
