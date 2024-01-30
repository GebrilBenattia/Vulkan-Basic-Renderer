#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

class RHI;

class VulkanInstance
{
private:

public:

	RHI& RHIInstance;

	VkInstance instance = nullptr;

	VulkanInstance(RHI& _RHI);

	void CreateInstance();

	std::vector<const char*> GetRequiredExtensions();

	void Cleanup();
};