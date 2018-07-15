#pragma once
#include <vulkan/vulkan.h>
#include <map>
#include "vks/VulkanBuffer.hpp"

#include "factory.h"


class UniMaterial {
public:
	UniMaterial() = default;
	
	VkPipeline m_Pipeline;
	VkPipelineLayout m_PipelineLayout;
	VkDescriptorSet m_DescriptorSet;
	VkDescriptorSetLayout m_DescriptorSetLayout;
	std::map<std::string, vks::Buffer> m_Buffers;

	std::string m_VertexShader = "";
	std::string m_FragmentShader = "";

	virtual void SetupPipeline(VkGraphicsPipelineCreateInfo& pipelineCreateInfo) = 0;

};


class PlanetMaterial : public UniMaterial {
public:
	void SetupPipeline(VkGraphicsPipelineCreateInfo& pipelineCreateInfo) override;
	PlanetMaterial() = default;
	PlanetMaterial(std::string name);

private:
	std::string m_Name;
};

using MaterialFactory = Factory<std::string, std::unique_ptr<UniMaterial>>::Initializer<std::string>;
