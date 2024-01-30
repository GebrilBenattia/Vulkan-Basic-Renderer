#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class RHI;

class Framebuffer
{
public:

	RHI& RHIInstance;

	Framebuffer(RHI& _RHI);

	void CreateFramebuffers();
};