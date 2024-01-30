#include "Swapchain.hpp"

#include "RHI.hpp"

Swapchain::Swapchain(RHI& _RHI)
	:RHIInstance(_RHI)
{
}

void Swapchain::CreateSwapchain()
{
    SwapChainSupportDetails SwapChainSupport = QuerySwapChainSupport(RHIInstance.RHIPhysicalDevice.physicalDevice);

    VkSurfaceFormatKHR SurfaceFormat = ChooseSwapSurfaceFormat(SwapChainSupport.Formats);
    VkPresentModeKHR PresentMode = ChooseSwapPresentMode(SwapChainSupport.PresentModes);
    VkExtent2D Extent = ChooseSwapExtent(SwapChainSupport.Capabilities);

    uint32_t ImageCount = SwapChainSupport.Capabilities.minImageCount + 1;
    if (SwapChainSupport.Capabilities.maxImageCount > 0 && ImageCount > SwapChainSupport.Capabilities.maxImageCount) {
        ImageCount = SwapChainSupport.Capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR CreateInfo{};
    CreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    CreateInfo.surface = RHIInstance.RHISurface.surface;

    CreateInfo.minImageCount = ImageCount;
    CreateInfo.imageFormat = SurfaceFormat.format;
    CreateInfo.imageColorSpace = SurfaceFormat.colorSpace;
    CreateInfo.imageExtent = Extent;
    CreateInfo.imageArrayLayers = 1;
    CreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices Indices = RHIInstance.RHIPhysicalDevice.FindQueueFamilies(RHIInstance.RHIPhysicalDevice.physicalDevice);
    uint32_t queueFamilyIndices[] = { Indices.GraphicsFamily.value(), Indices.PresentFamily.value() };

    if (Indices.GraphicsFamily != Indices.PresentFamily) {
        CreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        CreateInfo.queueFamilyIndexCount = 2;
        CreateInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else {
        CreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    CreateInfo.preTransform = SwapChainSupport.Capabilities.currentTransform;
    CreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    CreateInfo.presentMode = PresentMode;
    CreateInfo.clipped = VK_TRUE;

    if (vkCreateSwapchainKHR(RHIInstance.RHILogicalDevice.device, &CreateInfo, nullptr, &swapChain) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(RHIInstance.RHILogicalDevice.device, swapChain, &ImageCount, nullptr);
    SwapChainImages.resize(ImageCount);
    vkGetSwapchainImagesKHR(RHIInstance.RHILogicalDevice.device, swapChain, &ImageCount, SwapChainImages.data());

    SwapChainImageFormat = SurfaceFormat.format;
    SwapChainExtent = Extent;
}

void Swapchain::Cleanup()
{
    vkDestroyImageView(RHIInstance.RHILogicalDevice.device, RHIInstance.ColorImageView, nullptr);
    vkDestroyImage(RHIInstance.RHILogicalDevice.device, RHIInstance.ColorImage, nullptr);
    vkFreeMemory(RHIInstance.RHILogicalDevice.device, RHIInstance.ColorImageMemory, nullptr);

    vkDestroyImageView(RHIInstance.RHILogicalDevice.device, RHIInstance.DepthImageView, nullptr);
    vkDestroyImage(RHIInstance.RHILogicalDevice.device, RHIInstance.DepthImage, nullptr);
    vkFreeMemory(RHIInstance.RHILogicalDevice.device, RHIInstance.DepthImageMemory, nullptr);

    for (size_t i = 0; i < SwapChainFramebuffers.size(); i++) {
        vkDestroyFramebuffer(RHIInstance.RHILogicalDevice.device, SwapChainFramebuffers[i], nullptr);
    }

    for (size_t i = 0; i < SwapChainImageViews.size(); i++) {
        vkDestroyImageView(RHIInstance.RHILogicalDevice.device, SwapChainImageViews[i], nullptr);
    }

    vkDestroySwapchainKHR(RHIInstance.RHILogicalDevice.device, swapChain, nullptr);
}

VkExtent2D Swapchain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& _Capabilities)
{
    if (_Capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return _Capabilities.currentExtent;
    }
    else {
        int width, height;
        glfwGetFramebufferSize(RHIInstance.AppWindow.window, &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, _Capabilities.minImageExtent.width, _Capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, _Capabilities.minImageExtent.height, _Capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

VkPresentModeKHR Swapchain::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& _AvailablePresentModes)
{
    for (const auto& availablePresentMode : _AvailablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkSurfaceFormatKHR Swapchain::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& _AvailableFormats)
{
    for (const auto& availableFormat : _AvailableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return _AvailableFormats[0];
}

SwapChainSupportDetails Swapchain::QuerySwapChainSupport(VkPhysicalDevice _Device)
{
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_Device, RHIInstance.RHISurface.surface, &details.Capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(_Device, RHIInstance.RHISurface.surface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.Formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(_Device, RHIInstance.RHISurface.surface, &formatCount, details.Formats.data());
    }

    uint32_t PresentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(_Device, RHIInstance.RHISurface.surface, &PresentModeCount, nullptr);

    if (PresentModeCount != 0) {
        details.PresentModes.resize(PresentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(_Device, RHIInstance.RHISurface.surface, &PresentModeCount, details.PresentModes.data());
    }

    return details;
}

void Swapchain::RecreateSwapChain()
{
    int Width = 0, Height = 0;
    while (Width == 0 || Height == 0) {
        glfwGetFramebufferSize(RHIInstance.AppWindow.window, &Width, &Height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(RHIInstance.RHILogicalDevice.device);

    Cleanup();

    CreateSwapchain();
    RHIInstance.RHIImageView.CreateImageViews();
    RHIInstance.CreateColorResources();
    RHIInstance.CreateDepthResources();
    RHIInstance.RHIFramebuffer.CreateFramebuffers();
}
