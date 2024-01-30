#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class RHI;

class GraphicPipeline
{
private:

public:

	RHI& RHIInstance;

	VkPipeline graphicsPipeline;

	GraphicPipeline(RHI& _RHI);

	void CreateGraphicsPipeline();

	void Cleanup();
};