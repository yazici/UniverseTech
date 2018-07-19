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
	
	VkPipeline m_Pipeline;
	VkPipelineLayout m_PipelineLayout;
	VkDescriptorSet m_DescriptorSet;
	VkDescriptorSetLayout m_DescriptorSetLayout;
	VkDescriptorPool m_DescriptorPool;

	std::map<std::string, vks::Buffer> m_Buffers;


	std::string m_VertexShader = "";
	std::string m_FragmentShader = "";

	virtual void SetupMaterial(VkGraphicsPipelineCreateInfo& pipelineCreateInfo) = 0;
	virtual void AddToCommandBuffer(VkCommandBuffer& cmdBuffer) const = 0;
};


class PlanetMaterial : public UniMaterial {
public:

	// Vertex layout for the models
	vks::VertexLayout m_VertexLayout = vks::VertexLayout({
		vks::VERTEX_COMPONENT_POSITION
	});

	void SetupMaterial(VkGraphicsPipelineCreateInfo& pipelineCreateInfo) override;
	void AddToCommandBuffer(VkCommandBuffer& cmdBuffer) const override;

	std::shared_ptr<vks::Buffer> GetUniformBuffer() { return m_UniformBuffer; }
	std::shared_ptr<vks::Buffer> GetVertexBuffer() { return m_VertexBuffer; }
	std::shared_ptr<vks::Buffer> GetIndexBuffer() { return m_IndexBuffer; }
	std::vector<std::shared_ptr<vks::Texture>> m_Textures;

	void SetBuffer(std::string name, std::shared_ptr<vks::Buffer> buffer);
	

	void SetIndexCount(uint32_t count = 0) { m_IndexCount = count; }

private:
	std::string m_Name;
	std::shared_ptr<vks::Buffer> m_UniformBuffer;
	std::shared_ptr<vks::Buffer> m_VertexBuffer;
	std::shared_ptr<vks::Buffer> m_IndexBuffer;
	uint32_t m_IndexCount;

public:
	PlanetMaterial() = default;
	PlanetMaterial(std::string name);

};

using MaterialFactory = Factory<std::string, std::shared_ptr<UniMaterial>>::Initializer<std::string>;
