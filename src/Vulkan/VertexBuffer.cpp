#include "VertexBuffer.hpp"

#include "RHI.hpp"

VertexBuffer::VertexBuffer(RHI& _RHI)
	:RHIInstance(_RHI)
{
}

void VertexBuffer::CreateVertexBuffer()
{
    VkDeviceSize BufferSize = sizeof(RHIInstance.Vertices[0]) * RHIInstance.Vertices.size();

    VkBuffer StagingBuffer;
    VkDeviceMemory StagingBufferMemory;
    RHIInstance.RHICommandBuffer.CreateBuffer(BufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, StagingBuffer, StagingBufferMemory);

    void* Data;
    vkMapMemory(RHIInstance.RHILogicalDevice.device, StagingBufferMemory, 0, BufferSize, 0, &Data);
    memcpy(Data, RHIInstance.Vertices.data(), (size_t)BufferSize);
    vkUnmapMemory(RHIInstance.RHILogicalDevice.device, StagingBufferMemory);

    RHIInstance.RHICommandBuffer.CreateBuffer(BufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

    RHIInstance.RHICommandBuffer.CopyBuffer(StagingBuffer, vertexBuffer, BufferSize);

    vkDestroyBuffer(RHIInstance.RHILogicalDevice.device, StagingBuffer, nullptr);
    vkFreeMemory(RHIInstance.RHILogicalDevice.device, StagingBufferMemory, nullptr);
}

void VertexBuffer::Cleanup()
{
    vkDestroyBuffer(RHIInstance.RHILogicalDevice.device, vertexBuffer, nullptr);
    vkFreeMemory(RHIInstance.RHILogicalDevice.device, vertexBufferMemory, nullptr);
}
