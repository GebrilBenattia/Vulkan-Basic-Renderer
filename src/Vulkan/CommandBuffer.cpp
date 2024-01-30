#include "CommandBuffer.hpp"

#include "RHI.hpp"

CommandBuffer::CommandBuffer(RHI& _RHI)
    :RHIInstance(_RHI)
{
}

void CommandBuffer::RecordCommandBuffer(VkCommandBuffer _CommandBuffer, uint32_t _ImageIndex)
{
    VkCommandBufferBeginInfo BeginInfo{};
    BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    BeginInfo.flags = 0;  //Optional
    BeginInfo.pInheritanceInfo = nullptr;  //Optional

    if (vkBeginCommandBuffer(_CommandBuffer, &BeginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo RenderPassInfo{};
    RenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    RenderPassInfo.renderPass = RHIInstance.RenderPass;
    RenderPassInfo.framebuffer = RHIInstance.RHISwapchain.SwapChainFramebuffers[_ImageIndex];

    RenderPassInfo.renderArea.offset = { 0, 0 };
    RenderPassInfo.renderArea.extent = RHIInstance.RHISwapchain.SwapChainExtent;

    std::array<VkClearValue, 2> ClearValues{};
    ClearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
    ClearValues[1].depthStencil = { 1.0f, 0 };

    RenderPassInfo.clearValueCount = static_cast<uint32_t>(ClearValues.size());
    RenderPassInfo.pClearValues = ClearValues.data();

    vkCmdBeginRenderPass(_CommandBuffer, &RenderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, RHIInstance.RHIGraphicPipeline.graphicsPipeline);

    VkViewport Viewport{};
    Viewport.x = 0.0f;
    Viewport.y = 0.0f;
    Viewport.width = static_cast<float>(RHIInstance.RHISwapchain.SwapChainExtent.width);
    Viewport.height = static_cast<float>(RHIInstance.RHISwapchain.SwapChainExtent.height);
    Viewport.minDepth = 0.0f;
    Viewport.maxDepth = 1.0f;
    vkCmdSetViewport(_CommandBuffer, 0, 1, &Viewport);

    VkRect2D Scissor{};
    Scissor.offset = { 0, 0 };
    Scissor.extent = RHIInstance.RHISwapchain.SwapChainExtent;
    vkCmdSetScissor(_CommandBuffer, 0, 1, &Scissor);

    vkCmdBindPipeline(_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, RHIInstance.RHIGraphicPipeline.graphicsPipeline);

    VkBuffer VertexBuffers[] = { RHIInstance.RHIVertexBuffer.vertexBuffer };
    VkDeviceSize Offsets[] = { 0 };
    vkCmdBindVertexBuffers(_CommandBuffer, 0, 1, VertexBuffers, Offsets);

    vkCmdBindIndexBuffer(_CommandBuffer, RHIInstance.RHIIndexBuffer.indexBuffer, 0, VK_INDEX_TYPE_UINT32);

    vkCmdBindDescriptorSets(_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, RHIInstance.PipelineLayout, 0, 1, &RHIInstance.DescriptorSets[RHIInstance.CurrentFrame], 0, nullptr);
    vkCmdDrawIndexed(_CommandBuffer, static_cast<uint32_t>(RHIInstance.Indices.size()), 1, 0, 0, 0);

    vkCmdEndRenderPass(_CommandBuffer);

    if (vkEndCommandBuffer(_CommandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void CommandBuffer::CreateCommandBuffers()
{
    RHIInstance.CommandBuffers.resize(RHIInstance.MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo AllocInfo{};
    AllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    AllocInfo.commandPool = RHIInstance.RHICommandPool.commandPool;
    AllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    AllocInfo.commandBufferCount = (uint32_t)RHIInstance.CommandBuffers.size();

    if (vkAllocateCommandBuffers(RHIInstance.RHILogicalDevice.device, &AllocInfo, RHIInstance.CommandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

void CommandBuffer::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    VkCommandBuffer CommandBuffer = BeginSingleTimeCommands();

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(CommandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    EndSingleTimeCommands(CommandBuffer);
}

void CommandBuffer::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(RHIInstance.RHILogicalDevice.device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(RHIInstance.RHILogicalDevice.device, buffer, &memRequirements);

    VkMemoryAllocateInfo AllocInfo{};
    AllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    AllocInfo.allocationSize = memRequirements.size;
    AllocInfo.memoryTypeIndex = RHIInstance.RHIPhysicalDevice.FindMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(RHIInstance.RHILogicalDevice.device, &AllocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(RHIInstance.RHILogicalDevice.device, buffer, bufferMemory, 0);
}

VkCommandBuffer CommandBuffer::BeginSingleTimeCommands()
{
    VkCommandBufferAllocateInfo AllocInfo{};
    AllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    AllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    AllocInfo.commandPool = RHIInstance.RHICommandPool.commandPool;
    AllocInfo.commandBufferCount = 1;

    VkCommandBuffer CommandBuffer;
    vkAllocateCommandBuffers(RHIInstance.RHILogicalDevice.device, &AllocInfo, &CommandBuffer);

    VkCommandBufferBeginInfo BeginInfo{};
    BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    BeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(CommandBuffer, &BeginInfo);

    return CommandBuffer;
}

void CommandBuffer::EndSingleTimeCommands(VkCommandBuffer _CommandBuffer)
{
    vkEndCommandBuffer(_CommandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &_CommandBuffer;

    vkQueueSubmit(RHIInstance.GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(RHIInstance.GraphicsQueue);

    vkFreeCommandBuffers(RHIInstance.RHILogicalDevice.device, RHIInstance.RHICommandPool.commandPool, 1, &_CommandBuffer);
}

void CommandBuffer::CopyBufferToImage(VkBuffer _Buffer, VkImage _Image, uint32_t _Width, uint32_t _Height)
{
    VkCommandBuffer CommandBuffer = BeginSingleTimeCommands();

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = {
        _Width,
        _Height,
        1
    };

    vkCmdCopyBufferToImage(CommandBuffer, _Buffer, _Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    EndSingleTimeCommands(CommandBuffer);
}

void CommandBuffer::Cleanup()
{
}
