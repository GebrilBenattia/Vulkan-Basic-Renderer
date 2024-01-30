#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class RHI;

class ImageView
{
private:

public:

	RHI& RHIInstance;

	ImageView(RHI& _RHI);

	void CreateImageViews();

	VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);

	void Cleanup();
};