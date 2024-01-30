#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class RHI;

class VertexBuffer
{
public:

	RHI& RHIInstance;

	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;

	VertexBuffer(RHI& _RHI);

	void CreateVertexBuffer();

	void Cleanup();
};