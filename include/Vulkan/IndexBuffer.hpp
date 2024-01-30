#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class RHI;

class IndexBuffer
{
public:

	RHI& RHIInstance;

	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;

	IndexBuffer(RHI& _RHI);

	void CreateIndexBuffer();

	void Cleanup();
};