#include "VulkanComputePipeline.h"

#include <stdexcept>

#include "VulkanDevice.h"
#include "VulkanUtils.h"

my_vulkan::VulkanComputePipeline::VulkanComputePipeline(const std::shared_ptr<VulkanDevice>& device)
{
	createComputePipeline(device);
}

void my_vulkan::VulkanComputePipeline::createComputePipeline(const std::shared_ptr<VulkanDevice>& device)
{
	auto computeShader = VulkanUtils::readFile("shaders/compute.spv");
	auto computeShaderModule = VulkanUtils::createShaderModule(computeShader, device->getLogicalDevice());

	VkPipelineShaderStageCreateInfo computeShaderStageCreateInfo{};
	computeShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	computeShaderStageCreateInfo.module = computeShaderModule;
	computeShaderStageCreateInfo.pName = "main";
	computeShaderStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;

	VkComputePipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	pipelineInfo.layout = computePipelineLayout;
	pipelineInfo.stage = computeShaderStageCreateInfo;

	if(vkCreateComputePipelines(device->getLogicalDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &computePipeline) != VK_SUCCESS)
		throw std::runtime_error("failed to create compute pipeline!");

	auto setLayout = VulkanUtils::createDescriptorSetLayout(device->getLogicalDevice(), VulkanDescriptorFor::COMPUTE_SHADER_UNIFORM_BUFFER);

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &setLayout;

	if (vkCreatePipelineLayout(device->getLogicalDevice(), &pipelineLayoutInfo, nullptr, &computePipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create compute pipeline layout!");
	}

	vkDestroyShaderModule(device->getLogicalDevice(), computeShaderModule, nullptr);

}
