#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class RHI;

class LogicalDevice
{
public:

	RHI& RHIInstance;

	VkDevice device = nullptr;

	LogicalDevice(RHI& _RHI);

	void CreateLogicalDevice();

	void Cleanup();
};