#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class RHI;

class CommandPool
{
private:

public:

	RHI& RHIInstance;

	VkCommandPool commandPool;

	CommandPool(RHI& _RHI);

	void CreateCommandPool();

	void Cleanup();
};