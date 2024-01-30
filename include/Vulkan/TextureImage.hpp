#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class RHI;

class TextureImage
{
private:

public:

	RHI& RHIInstance;

	VkImage image;
	VkDeviceMemory textureImageMemory;

	VkImageView textureImageView;
	VkSampler textureSampler;

	uint32_t MipLevels;

	TextureImage(RHI& _RHI);

	void CreateTextureImage();

	void CreateTextureImageView();
	void CreateTextureSampler();

	void GenerateMipmaps(VkImage _Image, VkFormat _ImageFormat, int32_t _TexWidth, int32_t _TexHeight, uint32_t _MipLevels);

	void Cleanup();
};