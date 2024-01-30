#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class RHI;

class Framebuffer
{
private:

public:

	RHI& RHIInstance;

	Framebuffer(RHI& _RHI);

	void CreateFramebuffers();

	void Cleanup();
};