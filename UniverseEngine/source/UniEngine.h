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

#define VERTEX_BUFFER_BIND_ID 0
#define ENABLE_VALIDATION false
// todo: check if hardware supports sample number (or select max. supported)
#define SAMPLE_COUNT VK_SAMPLE_COUNT_8_BIT

class UniEngine : public VulkanExampleBase {
public:

	struct {
		struct {
			vks::Texture2D colorMap;
			vks::Texture2D normalMap;
		} model;
		struct {
			vks::Texture2D colorMap;
			vks::Texture2D normalMap;
		} floor;
	} textures;

	// Vertex layout for the models
	vks::VertexLayout vertexLayout = vks::VertexLayout({
		vks::VERTEX_COMPONENT_POSITION,
		vks::VERTEX_COMPONENT_UV,
		vks::VERTEX_COMPONENT_COLOR,
		vks::VERTEX_COMPONENT_NORMAL,
		vks::VERTEX_COMPONENT_TANGENT,
		});

	struct {
		vks::Model model;
		vks::Model floor;
		//vks::Model quad;
	} m_models;

	struct {
		VkPipelineVertexInputStateCreateInfo inputState;
		std::vector<VkVertexInputBindingDescription> bindingDescriptions;
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
	} vertices;

	struct {
		glm::mat4 projection;
		glm::mat4 model;
		glm::mat4 view;
		glm::vec4 instancePos[3];
	} uboVS, uboOffscreenVS;

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
	} uniformBuffers;

	struct {
		VkPipeline deferred;				// Deferred lighting calculation
		VkPipeline deferredNoMSAA;			// Deferred lighting calculation with explicit MSAA resolve
		VkPipeline offscreen;				// (Offscreen) scene rendering (fill G-Buffers)
		VkPipeline offscreenSampleShading;	// (Offscreen) scene rendering (fill G-Buffers) with sample shading rate enabled
		VkPipeline debug;					// G-Buffers debug display
	} pipelines;

	struct {
		VkPipelineLayout deferred;
		VkPipelineLayout offscreen;
	} pipelineLayouts;

	struct {
		VkDescriptorSet model;
		VkDescriptorSet floor;
	} descriptorSets;

	VkDescriptorSet m_descriptorSet;
	VkDescriptorSetLayout m_descriptorSetLayout;

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

	UniEngine();
	void getEnabledFeatures() override;
	void createAttachment(VkFormat format, VkImageUsageFlagBits usage, FrameBufferAttachment *attachment);
	void prepareOffscreenFramebuffer();
	void buildDeferredCommandBuffer();
	void buildCommandBuffers() override;
	void loadAssets();
	void setupVertexDescriptions();
	void setupDescriptorPool();
	void setupDescriptorSetLayout();
	void setupDescriptorSet();
	void preparePipelines();
	void prepareUniformBuffers();
	void updateUniformBuffersScreen();
	void updateUniformBufferDeferredMatrices();
	void updateUniformBufferDeferredLights();
	void draw();
	void prepare() override;
	void render() override;
	void viewChanged() override;
	void OnUpdateUIOverlay(vks::UIOverlay *overlay) override;
	~UniEngine();

	bool m_debugDisplay = false;
	bool m_useMSAA = true;
	bool m_useSampleShading = true;

	// One sampler for the frame buffer color attachments
	VkSampler m_colorSampler;

	VkCommandBuffer m_offScreenCmdBuffer = VK_NULL_HANDLE;

	// Semaphore used to synchronize between offscreen and final scene rendering
	VkSemaphore m_offscreenSemaphore = VK_NULL_HANDLE;
};
