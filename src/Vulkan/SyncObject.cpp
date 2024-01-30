#include "SyncObject.hpp"

#include "RHI.hpp"

SyncObject::SyncObject(RHI& _RHI)
    :RHIInstance(_RHI)
{
}

void SyncObject::CreateSyncObjects()
{
    RHIInstance.ImageAvailableSemaphores.resize(RHIInstance.MAX_FRAMES_IN_FLIGHT);
    RHIInstance.RenderFinishedSemaphores.resize(RHIInstance.MAX_FRAMES_IN_FLIGHT);
    RHIInstance.InFlightFences.resize(RHIInstance.MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo SemaphoreInfo{};
    SemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo FenceInfo{};
    FenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    FenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < RHIInstance.MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(RHIInstance.RHILogicalDevice.device, &SemaphoreInfo, nullptr, &RHIInstance.ImageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(RHIInstance.RHILogicalDevice.device, &SemaphoreInfo, nullptr, &RHIInstance.RenderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(RHIInstance.RHILogicalDevice.device, &FenceInfo, nullptr, &RHIInstance.InFlightFences[i]) != VK_SUCCESS) {

            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }
}

void SyncObject::Cleanup()
{
    for (size_t i = 0; i < RHIInstance.MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(RHIInstance.RHILogicalDevice.device, RHIInstance.RenderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(RHIInstance.RHILogicalDevice.device, RHIInstance.ImageAvailableSemaphores[i], nullptr);
        vkDestroyFence(RHIInstance.RHILogicalDevice.device, RHIInstance.InFlightFences[i], nullptr);
    }
}
