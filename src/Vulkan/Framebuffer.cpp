#include "Framebuffer.hpp"

#include "RHI.hpp"

Framebuffer::Framebuffer(RHI& _RHI)
    :RHIInstance(_RHI)
{
}

void Framebuffer::CreateFramebuffers()
{
    RHIInstance.RHISwapchain.SwapChainFramebuffers.resize(RHIInstance.RHISwapchain.SwapChainImageViews.size());

    for (size_t i = 0; i < RHIInstance.RHISwapchain.SwapChainImageViews.size(); i++) {
        std::array<VkImageView, 3> attachments = {
        RHIInstance.ColorImageView,
        RHIInstance.DepthImageView,
        RHIInstance.RHISwapchain.SwapChainImageViews[i]
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = RHIInstance.RenderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = RHIInstance.RHISwapchain.SwapChainExtent.width;
        framebufferInfo.height = RHIInstance.RHISwapchain.SwapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(RHIInstance.RHILogicalDevice.device, &framebufferInfo, nullptr, &RHIInstance.RHISwapchain.SwapChainFramebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

void Framebuffer::Cleanup()
{
}
