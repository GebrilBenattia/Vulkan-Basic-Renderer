#include "ValidationLayers.hpp"

#include "RHI.hpp"

ValidationLayer::ValidationLayer(RHI& _RHI)
    :RHIInstance(_RHI)
{
}

void ValidationLayer::SetupDebugMessenger()
{
    if (!EnableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT CreateInfo;
    PopulateDebugMessengerCreateInfo(CreateInfo);

    if (CreateDebugUtilsMessengerEXT(RHIInstance.RHIInstance.instance, &CreateInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}

VkResult ValidationLayer::CreateDebugUtilsMessengerEXT(VkInstance _Instance, const VkDebugUtilsMessengerCreateInfoEXT* _pCreateInfo, const VkAllocationCallbacks* _pAllocator, VkDebugUtilsMessengerEXT* _pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(_Instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(_Instance, _pCreateInfo, _pAllocator, _pDebugMessenger);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void ValidationLayer::DestroyDebugUtilsMessengerEXT(VkInstance _Instance, VkDebugUtilsMessengerEXT _DebugMessenger, const VkAllocationCallbacks* _pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(_Instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(_Instance, _DebugMessenger, _pAllocator);
    }
}

void ValidationLayer::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& _CreateInfo)
{
    _CreateInfo = {};
    _CreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    _CreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    _CreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    _CreateInfo.pfnUserCallback = DebugCallback;
}

VKAPI_ATTR VkBool32 VKAPI_CALL ValidationLayer::DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{

    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

bool ValidationLayer::CheckValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : ValidationLayers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    return true;
}

void ValidationLayer::Cleanup()
{
    if (EnableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(RHIInstance.RHIInstance.instance, debugMessenger, nullptr);
    }
}
