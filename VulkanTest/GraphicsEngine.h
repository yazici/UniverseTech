#pragma once

#ifndef GRAPHICSENGINE_H
#define GRAPHICSENGINE_H

#include "VulkanCommon.h"

#include "Camera.h"
#include "InputManager.h"
#include "Scene.h"
#include "Vertex.h"

class VulkanWinApp;

struct UniformBufferObject {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};


class GraphicsEngine {
public:
	GraphicsEngine(VulkanWinApp *app) { m_App = app; }
	~GraphicsEngine();
	Camera camera;

	void Init();
	void OneFrame();
	void cleanup();
	void recreate();
	void Setup();

	void cleanupSwapChain();
	void setDevice(VkDevice &d);
	void setExtent(VkExtent2D extent);

	std::vector<VkImage> m_SwapChainImages;
	std::vector<VkImageView> m_SwapChainImageViews;
	VkFormat m_SwapChainImageFormat;
	VkQueue m_GraphicsQueue;
	VkQueue m_PresentQueue;

private:
	VulkanWinApp * m_App = nullptr;
	VkBuffer m_VertexBuffer;
	VkDeviceMemory m_VertexBufferMemory;
	VkBuffer m_IndexBuffer;
	VkDeviceMemory m_IndexBufferMemory;
	VkBuffer m_UniformBuffer;
	VkDeviceMemory m_UniformBufferMemory;
	std::vector<VkDescriptorSet> m_DescriptorSets;
	VkRenderPass m_RenderPass;
	VkDescriptorSetLayout m_DescriptorSetLayout;
	VkPipelineLayout m_PipelineLayout;
	VkPipeline m_GraphicsPipeline;
	std::vector<VkFramebuffer> m_SwapChainFramebuffers;
	VkCommandPool m_CommandPool;
	VkDescriptorPool m_DescriptorPool;
	std::vector<VkCommandBuffer> m_CommandBuffers;
	std::vector<VkSemaphore> m_ImageAvailableSemaphores;
	std::vector<VkSemaphore> m_RenderFinishedSemaphores;
	std::vector<VkFence> m_InFlightFences;
	size_t m_CurrentFrame = 0;
	std::vector<uint32_t> m_MipLevels;
	std::vector<VkImage> m_TextureImages;
	VkDeviceMemory m_TextureImageMemory;
	std::vector<VkImageView> m_TextureImageViews;
	std::vector<VkSampler> m_TextureSamplers;
	VkImage m_DepthImage;
	VkDeviceMemory m_DepthImageMemory;
	VkImageView m_DepthImageView;
	InputManager m_InputManager;
	float m_FrameTime = 0.f;
	Scene m_CurrentScene;

	void setupInput();
	void handleKeyboardInput(GLFWwindow* w, int key, int scancode, int action, int mods);
	void handleMouseInput(GLFWwindow* w, double xpos, double ypos);
	void setupCamera();
	void createTextureImages();
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void generateMipmaps(VkImage image, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
	void createTextureImageViews();
	void createTextureSamplers();
	void createVertexBuffer();
	void createIndexBuffer();
	size_t getDynamicAlignment();
	void createUniformBuffers();
	void createRenderPass();
	void createDescriptorSetLayout();
	void createGraphicsPipeline();
	VkShaderModule createShaderModule(const std::vector<char>& code);
	void createCommandBuffers();
	void updateTime();
	void updateUniformBuffer();
	void drawFrame();
	VkCommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);

	void createFramebuffers();
	void createCommandPool();
	void createSyncObjects();
	void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
	void createDescriptorPool();
	void createDescriptorSets();
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	void createImageViews();
	void createDepthResources();
	VkFormat findDepthFormat();
	bool hasStencilComponent(VkFormat format);
	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);


	VkDevice m_Device;
	VkExtent2D m_SwapChainExtent;
	bool m_UseCameraModelMat = false;
};

#endif // !GRAPHICSENGINE_H