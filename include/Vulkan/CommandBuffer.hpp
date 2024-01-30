#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>

class RHI;

class CommandBuffer
{
private:

public:

	RHI& RHIInstance;

	CommandBuffer(RHI& _App);

	void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    void CreateCommandBuffers();

	void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

	void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

    VkCommandBuffer BeginSingleTimeCommands();

    void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

	void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

	void Cleanup();
};