#include "CommandPool.hpp"

#include "RHI.hpp"

CommandPool::CommandPool(RHI& _RHI)
	:RHIInstance(_RHI)
{
}

void CommandPool::CreateCommandPool()
{
    QueueFamilyIndices QueueFamilyIndices = RHIInstance.RHIPhysicalDevice.FindQueueFamilies(RHIInstance.RHIPhysicalDevice.physicalDevice);

    VkCommandPoolCreateInfo PoolInfo{};
    PoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    PoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    PoolInfo.queueFamilyIndex = QueueFamilyIndices.GraphicsFamily.value();

    if (vkCreateCommandPool(RHIInstance.RHILogicalDevice.device, &PoolInfo, nullptr, &commandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }
}

void CommandPool::Cleanup()
{
    vkDestroyCommandPool(RHIInstance.RHILogicalDevice.device, commandPool, nullptr);
}
