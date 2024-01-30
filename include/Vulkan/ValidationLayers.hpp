#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>

#include <vector>

class RHI;

class ValidationLayer
{
public:

	RHI& RHIInstance;

	VkDebugUtilsMessengerEXT debugMessenger = nullptr;

	const std::vector<const char*> ValidationLayers = {
	"VK_LAYER_KHRONOS_validation"
	};

	ValidationLayer(RHI& _RHI);

    void SetupDebugMessenger();
	
	VkResult CreateDebugUtilsMessengerEXT(VkInstance _Instance, const VkDebugUtilsMessengerCreateInfoEXT* _pCreateInfo, const VkAllocationCallbacks* _pAllocator, VkDebugUtilsMessengerEXT* _pDebugMessenger);

	void DestroyDebugUtilsMessengerEXT(VkInstance _Instance, VkDebugUtilsMessengerEXT _DebugMessenger, const VkAllocationCallbacks* _pAllocator);

    void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& _CreateInfo);

    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT _MessageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT _MessageType,
        const VkDebugUtilsMessengerCallbackDataEXT* _pCallbackData,
        void* _pUserData);

    bool CheckValidationLayerSupport();

	void Cleanup();
};