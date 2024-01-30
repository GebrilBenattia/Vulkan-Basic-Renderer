#include "IndexBuffer.hpp"

#include "RHI.hpp"

IndexBuffer::IndexBuffer(RHI& _RHI)
    :RHIInstance(_RHI)
{
}

void IndexBuffer::CreateIndexBuffer()
{
    VkDeviceSize bufferSize = sizeof(RHIInstance.Indices[0]) * RHIInstance.Indices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    RHIInstance.RHICommandBuffer.CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(RHIInstance.RHILogicalDevice.device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, RHIInstance.Indices.data(), (size_t)bufferSize);
    vkUnmapMemory(RHIInstance.RHILogicalDevice.device, stagingBufferMemory);

    RHIInstance.RHICommandBuffer.CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

    RHIInstance.RHICommandBuffer.CopyBuffer(stagingBuffer, indexBuffer, bufferSize);

    vkDestroyBuffer(RHIInstance.RHILogicalDevice.device, stagingBuffer, nullptr);
    vkFreeMemory(RHIInstance.RHILogicalDevice.device, stagingBufferMemory, nullptr);
}

void IndexBuffer::Cleanup()
{
    vkDestroyBuffer(RHIInstance.RHILogicalDevice.device, indexBuffer, nullptr);
    vkFreeMemory(RHIInstance.RHILogicalDevice.device, indexBufferMemory, nullptr);
}
