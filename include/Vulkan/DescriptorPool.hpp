#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class RHI;

class DescriptorPool
{
public:

	RHI& RHIInstance;

	VkDescriptorPool descriptorPool;

	DescriptorPool(RHI& _RHI);

	void CreateDescriptorPool();

	void Cleanup();
};