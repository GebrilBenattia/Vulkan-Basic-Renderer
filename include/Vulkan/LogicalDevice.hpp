#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class RHI;

class LogicalDevice
{
private:

public:

	RHI& RHIInstance;

	VkDevice device = nullptr;

	LogicalDevice(RHI& _RHI);

	void CreateLogicalDevice();

	void Cleanup();
};