#include "VulkanRenderer.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include <stdexcept>
#include "Vertex.h"
#include "VulkanContext.h"
#include "VulkanDescriptors.h"
#include "VulkanDevice.h"
#include "VulkanComputePipeline.h"
#include "VulkanGraphicsPipeline.h"
#include "VulkanDepthResources.h"
#include "VulkanSwapChain.h"
#include "VulkanUniformBuffers.h"
#include "Object.h"
#include "Texture.h"
#include "Model.h"
#include "VulkanImage.h"
#include "VulkanUtils.h"
#include <imconfig.h>
#include "ImguiAPI.h"

my_vulkan::VulkanRenderer::VulkanRenderer(my_vulkan::VulkanContext* context) : maxRenderImages(MAX_RENDER_IMAGES), currentFrame(0)
{
	VkFormat colorFormat = context->swapChain->getSwapChainFormat().format;
	colorRecources = std::make_shared<VulkanImage>(context->device, context->swapChain->getSwapChainExtent().width, context->swapChain->getSwapChainExtent().height, 1, 1, 1, VK_IMAGE_TYPE_2D,
		colorFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_SHARING_MODE_EXCLUSIVE,
		context->device->getMsaaSamples(), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_COLOR_BIT);

	depthResources = std::make_shared<VulkanDepthResources>(context->device, context->swapChain->getSwapChainExtent(), context->commandPool);
	createFramebuffers(context->device->getLogicalDevice(), context->swapChain, context->graphicsPipeline->getRenderPass());
	createCommandBuffer(context->device->getLogicalDevice(), context->commandPool);
	createSynchronizationObjects(context->device->getLogicalDevice());
	
	clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
	clearValues[1].depthStencil = { 1.0f, 0 };
}

void my_vulkan::VulkanRenderer::createFramebuffers(const VkDevice& device, const std::shared_ptr<VulkanSwapChain> swapChain, const VkRenderPass& renderPass)
{
	frameBuffers.resize(swapChain->getImageViews().size());
	int i = 0;
	for (auto& frameBuffer : frameBuffers)
	{
		std::array<VkImageView, 3> attachments = { colorRecources->getImageView(), depthResources->getImageView(), swapChain->getImageViews()[i++] };
		VkFramebufferCreateInfo frameBufferCreateInfo{};
		frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frameBufferCreateInfo.width = swapChain->getSwapChainExtent().width;
		frameBufferCreateInfo.height = swapChain->getSwapChainExtent().height;
		frameBufferCreateInfo.attachmentCount = attachments.size();
		frameBufferCreateInfo.pAttachments = attachments.data();
		frameBufferCreateInfo.layers = 1;
		frameBufferCreateInfo.renderPass = renderPass;

		if (vkCreateFramebuffer(device, &frameBufferCreateInfo, nullptr, &frameBuffer) != VK_SUCCESS)
			throw std::runtime_error("failed to create frame buffer!");
	}
}

void my_vulkan::VulkanRenderer::createCommandBuffer(const VkDevice& device, VkCommandPool& commandPool)
{
	commandBuffers.resize(maxRenderImages);
	for(auto& commandBuffer : commandBuffers)
	{
		VkCommandBufferAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocateInfo.commandBufferCount = 1;
		allocateInfo.commandPool = commandPool;
		allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		if (vkAllocateCommandBuffers(device, &allocateInfo, &commandBuffer) != VK_SUCCESS)
			throw std::runtime_error("failed to allocate command buffer!");
	}
	for (auto& commandBuffer : computeCommandBuffers)
	{
		VkCommandBufferAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocateInfo.commandBufferCount = 1;
		allocateInfo.commandPool = commandPool;
		allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		if (vkAllocateCommandBuffers(device, &allocateInfo, &commandBuffer) != VK_SUCCESS)
			throw std::runtime_error("failed to allocate command buffer!");
	}
}

void my_vulkan::VulkanRenderer::createSynchronizationObjects(const VkDevice& device)
{
	
	VkSemaphoreCreateInfo semaphoreCreateInfo{};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceCreateInfo{};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = VkFenceCreateFlagBits::VK_FENCE_CREATE_SIGNALED_BIT;

	imageAvailableSemaphores.resize(maxRenderImages);
	renderFinishedSemaphores.resize(maxRenderImages);
	inFlightFences.resize(maxRenderImages);

	for(auto& imageAvailableSemaphore : imageAvailableSemaphores)
		if (vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS)
			throw std::runtime_error("failed to create imageAvailableSemaphores");

	for (auto& renderFinishedSemaphore : renderFinishedSemaphores)
		if (vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS)
			throw std::runtime_error("failed to create renderFinishedSemaphores");

	for (auto& inFlightFence : inFlightFences)
		if (vkCreateFence(device, &fenceCreateInfo, nullptr, &inFlightFence) != VK_SUCCESS)
			throw std::runtime_error("failed to create inFlightFences");
}

void my_vulkan::VulkanRenderer::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, const std::shared_ptr<VulkanGraphicsPipeline>& pipeline,
	const VkExtent2D& swapChainExtent, ImguiAPI* imgui, const std::vector<std::shared_ptr<Object>>& objects)
{

	VkCommandBufferBeginInfo commandBufferBeginInfo{};
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);

	VkFramebuffer frameBuffer = frameBuffers[imageIndex];

	VkRenderPassBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	beginInfo.renderPass = pipeline->getRenderPass();
	beginInfo.framebuffer = frameBuffer;
	beginInfo.clearValueCount = clearValues.size();
	beginInfo.pClearValues = clearValues.data();
	beginInfo.renderArea.extent = swapChainExtent;
	beginInfo.renderArea.offset = VkOffset2D{ 0, 0 };

	vkCmdBeginRenderPass(commandBuffer, &beginInfo, VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getGraphicsPipeline());
	VkViewport viewport{};
	viewport.width = swapChainExtent.width;
	viewport.height = swapChainExtent.height;
	viewport.maxDepth = 1.0f;
	viewport.minDepth = 0.0f;
	viewport.x = 0.0f;
	viewport.y = 0.0f;

	VkRect2D scissor{};
	scissor.extent = swapChainExtent;
	scissor.offset = VkOffset2D{ 0, 0 };

	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	for (const auto & object : objects)
	{
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getPipelineLayout(), 0, 1,
			&object->uniformBuffersDescriptors->getDescriptorSets().at(currentFrame), 0, nullptr);
		for (int i = 0; i != object->textures.size(); ++i)
		{
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getPipelineLayout(), 1, 1,
				&object->textures[i]->descriptor->getDescriptorSets().at(currentFrame), 0, nullptr);
			object->meshes[i]->Render(commandBuffer);
		}
	}

	imgui->updateImgui(commandBuffer, objects);

	vkCmdEndRenderPass(commandBuffer);
	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
		throw std::runtime_error("failed to record command buffer");
}

void my_vulkan::VulkanRenderer::recordComputeCommandBuffer(VkCommandBuffer commandBuffer, const std::shared_ptr<VulkanComputePipeline>& computePipeline, 
	const std::shared_ptr<VulkanDescriptors>& descriptors)
{
	VkCommandBufferBeginInfo beginInfo;
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline->getComputePipeline());

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline->getComputePipelineLayout(),
		0, 1, &descriptors->getDescriptorSets()[currentFrame], 0, 0);

	vkCmdDispatch(commandBuffer, PARTICLE_COUNT / 256, 1, 1);

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to record command buffer!");
	}
}

void my_vulkan::VulkanRenderer::draw(my_vulkan::VulkanContext* context, ImguiAPI* imgui, const std::vector<std::shared_ptr<Object>>& objects)
{
	VkSubmitInfo submitInfo{};

	vkWaitForFences(context->device->getLogicalDevice(), 1, &inFlightFences[currentFrame], VK_FALSE, UINT64_MAX);

	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(context->device->getLogicalDevice(), context->swapChain->getSwapChain(), UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		recreateSwapChain(context->swapChain, context->wind.window, context->device, context->surface, context->graphicsPipeline->getRenderPass(), context->commandPool);
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		throw std::runtime_error("failed to acquire next image");
	
	}
	recordCommandBuffer(commandBuffers[currentFrame], imageIndex, context->graphicsPipeline, context->swapChain->getSwapChainExtent(), imgui, objects);

	vkResetFences(context->device->getLogicalDevice(), 1, &inFlightFences[currentFrame]);

	VkPipelineStageFlags waitDstStageMask = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };

	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers[currentFrame];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &renderFinishedSemaphores[currentFrame];
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = &waitDstStageMask;


	vkQueueSubmit(context->device->getGraphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]);

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &context->swapChain->getSwapChain();
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &renderFinishedSemaphores[currentFrame];

	result = vkQueuePresentKHR(context->device->getPresentQueue(), & presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || context->wind.framebufferResized)
	{
		context->wind.framebufferResized = false;
		recreateSwapChain(context->swapChain, context->wind.window, context->device, context->surface, context->graphicsPipeline->getRenderPass(), context->commandPool);
	}
	else if(result != VK_SUCCESS)
		throw std::runtime_error("failed to present image!");

	currentFrame = (currentFrame + 1) % maxRenderImages;
}

void my_vulkan::VulkanRenderer::recreateSwapChain(std::shared_ptr<VulkanSwapChain> swapChain, GLFWwindow* window,
	const std::shared_ptr<VulkanDevice>& device, const VkSurfaceKHR& surface, const VkRenderPass& renderPass, VkCommandPool& commandPool)
{
	vkDeviceWaitIdle(device->getLogicalDevice());
	for (size_t i = 0; i < frameBuffers.size(); i++) {
		vkDestroyFramebuffer(device->getLogicalDevice(), frameBuffers[i], nullptr);
	}
	swapChain->recreateSwapChain(window, device->getLogicalDevice(), device->getPhysicalDevice(), surface);
	colorRecources = std::make_shared<VulkanImage>(device, swapChain->getSwapChainExtent().width, swapChain->getSwapChainExtent().height, 1, 1, 1, VK_IMAGE_TYPE_2D, VkFormat::VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_SHARING_MODE_EXCLUSIVE, device->getMsaaSamples(),
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_COLOR_BIT);

	depthResources->createDepthBuffer(device, swapChain->getSwapChainExtent(), commandPool);
	createFramebuffers(device->getLogicalDevice(), swapChain, renderPass);
}

void my_vulkan::VulkanRenderer::destroyRenderer(const VkDevice& device)
{
	for (int i = 0; i != maxRenderImages; ++i)
	{
		vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
		vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
		vkDestroyFence(device, inFlightFences[i], nullptr);
	}
	for (auto& framebuffer : frameBuffers)
		vkDestroyFramebuffer(device, framebuffer, nullptr);
}

my_vulkan::VulkanRenderer::~VulkanRenderer()
{

}
