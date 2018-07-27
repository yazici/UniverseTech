#pragma once
#include <vulkan/vulkan.h>
#include <map>
#include "vks/VulkanBuffer.hpp"
#include "factory.h"
#include "vks/VulkanTexture.hpp"
#include "vks/VulkanModel.hpp"


class UniMaterial {
public:

	struct {
		VkPipelineVertexInputStateCreateInfo inputState;
		std::vector<VkVertexInputBindingDescription> bindingDescriptions;
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
	} m_VertexDescription;

	// Vertex layout for the models
	vks::VertexLayout m_VertexLayout = vks::VertexLayout({
		vks::VERTEX_COMPONENT_POSITION,
		vks::VERTEX_COMPONENT_UV,
		vks::VERTEX_COMPONENT_COLOR,
		vks::VERTEX_COMPONENT_NORMAL,
		vks::VERTEX_COMPONENT_TANGENT,
		});

	UniMaterial() = default;
	virtual void Destroy();
	virtual ~UniMaterial() { Destroy(); }
	
	VkPipeline m_Pipeline;
	VkPipelineLayout m_PipelineLayout;
	VkPipeline m_OceanPipeline;
	VkPipelineLayout m_OceanPipelineLayout;
	VkDescriptorSet m_DescriptorSet;
	VkDescriptorSetLayout m_DescriptorSetLayout;
	VkDescriptorPool m_DescriptorPool;

	std::map<std::string, std::shared_ptr<vks::Buffer>> m_Buffers;
	std::shared_ptr<vks::Buffer> GetBuffer(std::string buffer);
	void SetBuffer(std::string name, std::shared_ptr<vks::Buffer> buffer);


	std::map<std::string, std::string> m_Shaders;
	std::string GetShader(std::string name) { return m_Shaders.at(name); }
	void SetShader(std::string name, std::string shader) { m_Shaders[name] = shader; }

	virtual void SetupMaterial(VkGraphicsPipelineCreateInfo& pipelineCreateInfo) = 0;
	virtual void AddToCommandBuffer(VkCommandBuffer& cmdBuffer) = 0;
};


class PlanetMaterial : public UniMaterial {
public:

	virtual ~PlanetMaterial() { Destroy(); }

	// Vertex layout for the models
	vks::VertexLayout m_VertexLayout = vks::VertexLayout({
		vks::VERTEX_COMPONENT_POSITION
	});

	void SetupMaterial(VkGraphicsPipelineCreateInfo& pipelineCreateInfo) override;
	void AddToCommandBuffer(VkCommandBuffer& cmdBuffer) override;

	std::vector<std::shared_ptr<vks::Texture>> m_Textures;

	void SetIndexCount(uint32_t count = 0) { m_IndexCount = count; }

	void Destroy() override;

private:
	std::string m_Name;
	uint32_t m_IndexCount;

	bool m_RenderOcean = false;
public:
	PlanetMaterial() = default;
	PlanetMaterial(std::string name, bool hasOcean = false);

};

using MaterialFactory = Factory<std::string, std::shared_ptr<UniMaterial>>::Initializer<std::string, bool>;
