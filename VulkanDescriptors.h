#pragma once
#include <memory>
#include <vector>
#include <vulkan/vulkan.h>


namespace my_vulkan
{
	enum class VulkanDescriptorFor;
	class VulkanUniformBuffers;
	class Texture;
	class VulkanDevice;

	class VulkanDescriptors
	{
	public:
		VulkanDescriptors(const std::shared_ptr<VulkanDevice>& device, VkBuffer* uniformBuffers, Texture* texture, VulkanDescriptorFor layout_type);
		VulkanDescriptors(const std::shared_ptr<VulkanDevice>& device, VkBuffer* uniformBuffers, VkImageView& imageView, VkSampler& sampler, VulkanDescriptorFor layout_type);

		void createDescriptorSetLayout(const VkDevice& device, VulkanDescriptorFor layout_type);
		void DestroyVulkanDescriptor(const VkDevice& device);

		void createDescriptorPool(const VkDevice& device, VulkanDescriptorFor layout_type);
		void createDescriptorSets(const VkDevice& device, VkBuffer* uniformBuffers, Texture* texture, VulkanDescriptorFor layout_type);
		void createDescriptorSets(const VkDevice& device, VkBuffer* uniformBuffers, const VkImageView& imageView, const VkSampler& sampler, VulkanDescriptorFor layout_type);

		VkDescriptorSetLayout& getDescriptorSetLayout() { return descriptorSetLayout; }
		VkDescriptorPool& getDescriptorPool() { return descriptorPool; }
		std::vector<VkDescriptorSet>& getDescriptorSets() { return descriptorSets; }

	private:
		VkDescriptorSetLayout descriptorSetLayout;

		VkDescriptorPool descriptorPool;
		std::vector<VkDescriptorSet> descriptorSets;
	};
}

