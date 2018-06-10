#pragma once

#ifndef VULKANWINAPP_H
#define VULKANWINAPP_H

#include <iostream>
#include <stdexcept>
#include <functional>
#include <cstdlib>
#include <vector>
#include <cstring>
#include <set>
#include <algorithm>
#include <fstream>
#include <array>
#include <chrono>
#include <unordered_map>
#include <iomanip>

#include "VulkanCommon.h"
#include "GraphicsEngine.h"


const std::vector<const char*> validationLayers = {
	"VK_LAYER_LUNARG_standard_validation"
};

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

struct QueueFamilyIndices {
	int graphicsFamily = -1;
	int presentFamily = -1;

	bool isComplete() {
		return graphicsFamily >= 0 && presentFamily >= 0;
	}
};

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

class VulkanWinApp {

public:

	VulkanWinApp();
	~VulkanWinApp();

	void run() {
		initWindow();
		initVulkan();
		engine->Setup();
		mainLoop();
		cleanup();
	}

	GLFWwindow* GetWindow() { return window; }
	GraphicsEngine* engine = nullptr;
	VkDevice GetDevice() { return device; }
	VkPhysicalDevice GetPhysicalDevice();
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
	VkSwapchainKHR swapChain;
	void recreateSwapChain();

private:
	bool isMinimised = false;

	GLFWwindow * window = nullptr;
	VkInstance instance;
	VkDebugReportCallbackEXT callback;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device;
	VkSurfaceKHR surface;
	
	

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugReportFlagsEXT flags,
		VkDebugReportObjectTypeEXT objType,
		uint64_t obj,
		size_t location,
		int32_t code,
		const char* layerPrefix,
		const char* msg,
		void* userData);


	bool checkValidationLayerSupport();
	std::vector<const char*> getRequiredExtensions();
	void initWindow();
	void pickPhysicalDevice();
	bool isDeviceSuitable(VkPhysicalDevice device);
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	void initVulkan();
	
	void createSwapChain();
	void createLogicalDevice();
	void createSurface();
	void setupDebugCallback();
	void createInstance();
	void mainLoop();
	void cleanupSwapChain();
	void cleanup();
	
};

#endif // !VULKANWINAPP_H