#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vulkan/vulkan.h>
#include "vks/vulkanexamplebase.h"
#include "vks/VulkanBuffer.hpp"
#include "vks/VulkanTexture.hpp"
#include "vks/VulkanModel.hpp"

#include "UniModel.h"
#include "UniScene.h"

#define VERTEX_BUFFER_BIND_ID 0
#define INSTANCE_BUFFER_BIND_ID 1
#define ENABLE_VALIDATION false
// todo: check if hardware supports sample number (or select max. supported)
#define SAMPLE_COUNT VK_SAMPLE_COUNT_8_BIT

class UniEngine final : public VulkanExampleBase {

private:
	UniEngine();
	~UniEngine();
	UniEngine(const UniEngine&) = delete;
	UniEngine& operator=(const UniEngine&) = delete;
	UniEngine(UniEngine&&) = delete;
	UniEngine& operator=(UniEngine&&) = delete;


public:

	static UniEngine& GetInstance();

	// Vertex layout for the models
	vks::VertexLayout vertexLayout = vks::VertexLayout({
		vks::VERTEX_COMPONENT_POSITION,
		vks::VERTEX_COMPONENT_UV,
		vks::VERTEX_COMPONENT_COLOR,
		vks::VERTEX_COMPONENT_NORMAL,
		vks::VERTEX_COMPONENT_TANGENT,
		});

	std::vector<std::shared_ptr<UniModel>> m_models;

	struct {
		VkPipelineVertexInputStateCreateInfo inputState;
		std::vector<VkVertexInputBindingDescription> bindingDescriptions;
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
	} vertices;

	struct {
		glm::mat4 projection;
		glm::mat4 view;
		glm::mat4 model;
	} uboVS, uboOffscreenVS;

	struct UboModelMatDynamic {
		glm::mat4 *model = nullptr;
	} uboModelMatDynamic;


	struct Light {
		glm::vec4 position;
		glm::vec3 color;
		float radius;
	};

	struct {
		Light lights[6];
		glm::vec4 viewPos;
		glm::ivec2 windowSize;
	} uboFragmentLights;

	struct {
		vks::Buffer vsFullScreen;
		vks::Buffer vsOffscreen;
		vks::Buffer fsLights;
		vks::Buffer modelViews;
	} uniformBuffers;

	struct {
		VkPipeline deferred;				// Deferred lighting calculation
		VkPipeline deferredNoMSAA;			// Deferred lighting calculation with explicit MSAA resolve
		VkPipeline offscreen;				// (Offscreen) scene rendering (fill G-Buffers)
		VkPipeline offscreenSampleShading;	// (Offscreen) scene rendering (fill G-Buffers) with sample shading rate enabled
		VkPipeline debug;					// G-Buffers debug display
		VkPipeline offScreenPlanet;
	} pipelines;

	struct {
		VkPipelineLayout deferred;
		VkPipelineLayout offscreen;
		VkPipelineLayout planetOffscreen;
	} pipelineLayouts;

	VkDescriptorSet m_descriptorSet;
	VkDescriptorSet m_descriptorSetDynamic;
	VkDescriptorSetLayout m_descriptorSetLayout;
	VkDescriptorSetLayout m_descriptorSetLayoutPlanet;
	VkDescriptorSetLayout m_descriptorSetLayoutDynamic;

	// Framebuffer for offscreen rendering
	struct FrameBufferAttachment {
		VkImage image;
		VkDeviceMemory mem;
		VkImageView view;
		VkFormat format;
	};
	struct FrameBuffer {
		int32_t width, height;
		VkFramebuffer frameBuffer;
		FrameBufferAttachment position, normal, albedo;
		FrameBufferAttachment depth;
		VkRenderPass renderPass;
	} offScreenFrameBuf;


	void getEnabledFeatures() override;
	void createAttachment(VkFormat format, VkImageUsageFlagBits usage, FrameBufferAttachment *attachment);
	void prepareOffscreenFramebuffer();
	void buildDeferredCommandBuffer();
	void buildCommandBuffers() override;
	void loadAssets();
	void setupVertexDescriptions();
	void setupDescriptorPool();
	void setupDescriptorSetLayout();
	void setupDescriptorSets();
	void preparePipelines();
	size_t getDynamicAlignment();
	void prepareUniformBuffers();
	void updateUniformBuffersScreen();
	void updateUniformBufferDeferredMatrices();
	void updateUniformBufferDeferredLights();
	void updateDynamicUniformBuffers();
	void draw();
	void prepare() override;
	void render() override;
	void viewChanged() override;
	void OnUpdateUIOverlay(vks::UIOverlay *overlay) override;

	VkDevice GetDevice() { return device; }
	VkQueue GetQueue() { return queue; }
	void Shutdown();
	void buildPlanetCommandBuffer();
	bool m_debugDisplay = false;
	bool m_useMSAA = true;
	bool m_useSampleShading = true;

	// One sampler for the frame buffer color attachments
	VkSampler m_colorSampler;

	VkCommandBuffer m_offScreenCmdBuffer = VK_NULL_HANDLE;
	VkCommandBuffer m_planetCmdBuffer = VK_NULL_HANDLE;

	// Semaphore used to synchronize between offscreen and final scene rendering
	VkSemaphore m_offscreenSemaphore = VK_NULL_HANDLE;

	std::unique_ptr<UniScene> m_CurrentScene;


};
