#include "ImageView.hpp"

#include "RHI.hpp"

ImageView::ImageView(RHI& _RHI)
	:RHIInstance(_RHI)
{
}

void ImageView::CreateImageViews()
{
    RHIInstance.RHISwapchain.SwapChainImageViews.resize(RHIInstance.RHISwapchain.SwapChainImages.size());

    for (uint32_t i = 0; i < RHIInstance.RHISwapchain.SwapChainImages.size(); i++) {
        RHIInstance.RHISwapchain.SwapChainImageViews[i] = CreateImageView(RHIInstance.RHISwapchain.SwapChainImages[i], RHIInstance.RHISwapchain.SwapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    }
}

VkImageView ImageView::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels)
{
    VkImageViewCreateInfo ViewInfo{};
    ViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    ViewInfo.image = image;
    ViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ViewInfo.format = format;
    ViewInfo.subresourceRange.aspectMask = aspectFlags;
    ViewInfo.subresourceRange.baseMipLevel = 0;
    ViewInfo.subresourceRange.levelCount = mipLevels;
    ViewInfo.subresourceRange.baseArrayLayer = 0;
    ViewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (vkCreateImageView(RHIInstance.RHILogicalDevice.device, &ViewInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }

    return imageView;
}
