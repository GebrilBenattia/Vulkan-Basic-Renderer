#include "DescriptorPool.hpp"

#include "RHI.hpp"

DescriptorPool::DescriptorPool(RHI& _RHI)
	:RHIInstance(_RHI)
{
}

void DescriptorPool::CreateDescriptorPool()
{
    std::array<VkDescriptorPoolSize, 3> PoolSizes{};
    PoolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    PoolSizes[0].descriptorCount = static_cast<uint32_t>(RHIInstance.MAX_FRAMES_IN_FLIGHT);
    PoolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    PoolSizes[1].descriptorCount = static_cast<uint32_t>(RHIInstance.MAX_FRAMES_IN_FLIGHT);
    PoolSizes[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    PoolSizes[2].descriptorCount = static_cast<uint32_t>(RHIInstance.MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo PoolInfo{};
    PoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    PoolInfo.poolSizeCount = static_cast<uint32_t>(PoolSizes.size());
    PoolInfo.pPoolSizes = PoolSizes.data();
    PoolInfo.maxSets = static_cast<uint32_t>(RHIInstance.MAX_FRAMES_IN_FLIGHT);

    if (vkCreateDescriptorPool(RHIInstance.RHILogicalDevice.device, &PoolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void DescriptorPool::Cleanup()
{
    vkDestroyDescriptorPool(RHIInstance.RHILogicalDevice.device, descriptorPool, nullptr);
}
