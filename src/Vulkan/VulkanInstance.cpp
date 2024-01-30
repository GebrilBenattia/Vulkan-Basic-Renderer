#include "VulkanInstance.hpp"

#include "RHI.hpp"

VulkanInstance::VulkanInstance(RHI& _RHI)
    :RHIInstance(_RHI)
{
}

void VulkanInstance::CreateInstance()
{
    if (EnableValidationLayers && !RHIInstance.RHIValidationLayer.CheckValidationLayerSupport()) {
        throw std::runtime_error("validation layers requested, but not available!");
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    if (EnableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(RHIInstance.RHIValidationLayer.ValidationLayers.size());
        createInfo.ppEnabledLayerNames = RHIInstance.RHIValidationLayer.ValidationLayers.data();
    }
    else {
        createInfo.enabledLayerCount = 0;
    }

    auto extensions = GetRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (EnableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(RHIInstance.RHIValidationLayer.ValidationLayers.size());
        createInfo.ppEnabledLayerNames = RHIInstance.RHIValidationLayer.ValidationLayers.data();

        RHIInstance.RHIValidationLayer.PopulateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }
    else {
        createInfo.enabledLayerCount = 0;

        createInfo.pNext = nullptr;
    }


    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }
}

std::vector<const char*> VulkanInstance::GetRequiredExtensions()
{
    uint32_t GlfwExtensionCount = 0;
    const char** GlfwExtensions;
    GlfwExtensions = glfwGetRequiredInstanceExtensions(&GlfwExtensionCount);

    std::vector<const char*> Extensions(GlfwExtensions, GlfwExtensions + GlfwExtensionCount);

    if (EnableValidationLayers) {
        Extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return Extensions;
}

void VulkanInstance::Cleanup()
{
    vkDestroyInstance(RHIInstance.RHIInstance.instance, nullptr);
}
