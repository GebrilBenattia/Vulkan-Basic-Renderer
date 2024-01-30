#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class RHI;

class Surface
{
public:

	RHI& RHIInstance;

	VkSurfaceKHR surface = nullptr;

	Surface(RHI& _RHI);

	void CreateSurface();

	void Cleanup();

};