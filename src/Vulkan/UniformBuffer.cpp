#include "UniformBuffer.hpp"

#include "RHI.hpp"

UniformBuffer::UniformBuffer(RHI& _RHI)
    :RHIInstance(_RHI)
{
}

void UniformBuffer::CreateUniformBuffers()
{
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    RHIInstance.UniformBuffers.resize(RHIInstance.MAX_FRAMES_IN_FLIGHT);
    RHIInstance.UniformBuffersMemory.resize(RHIInstance.MAX_FRAMES_IN_FLIGHT);
    RHIInstance.UniformBuffersMapped.resize(RHIInstance.MAX_FRAMES_IN_FLIGHT);

    RHIInstance.LightUniformBuffers.resize(RHIInstance.MAX_FRAMES_IN_FLIGHT);
    RHIInstance.LightUniformBuffersMemory.resize(RHIInstance.MAX_FRAMES_IN_FLIGHT);
    RHIInstance.LightUniformBuffersMapped.resize(RHIInstance.MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < RHIInstance.MAX_FRAMES_IN_FLIGHT; i++) {
        RHIInstance.RHICommandBuffer.CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, RHIInstance.UniformBuffers[i], RHIInstance.UniformBuffersMemory[i]);
        RHIInstance.RHICommandBuffer.CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, RHIInstance.LightUniformBuffers[i], RHIInstance.LightUniformBuffersMemory[i]);

        vkMapMemory(RHIInstance.RHILogicalDevice.device, RHIInstance.UniformBuffersMemory[i], 0, bufferSize, 0, &RHIInstance.UniformBuffersMapped[i]);
        vkMapMemory(RHIInstance.RHILogicalDevice.device, RHIInstance.LightUniformBuffersMemory[i], 0, bufferSize, 0, &RHIInstance.LightUniformBuffersMapped[i]);
    }
}

void UniformBuffer::UpdateUniformBuffer(uint32_t _CurrentImage)
{
    static auto StartTime = std::chrono::high_resolution_clock::now();
    auto CurrentTime = std::chrono::high_resolution_clock::now();
    float Time = std::chrono::duration<float, std::chrono::seconds::period>(CurrentTime - StartTime).count();

    UniformBufferObject Ubo{};
    Ubo.Model = glm::rotate(glm::mat4(1.0f), Time * glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));;
    Ubo.View = RHIInstance.Cam.view;
    Ubo.Proj = RHIInstance.Cam.projection;
    Ubo.Proj[1][1] *= -1;

    PointLight Light{};
    Light.Pos = glm::vec3(0.f, 0.f, 0.f);
    Light.Ambient = glm::vec3(0.1f, 0.1f, 0.f);
    Light.Diffuse = glm::vec3(1.f, 1.f, 0.f);
    Light.Specular = glm::vec3(0.f, 1.f, 0.f);
    Light.CamPos = RHIInstance.Cam.Position;

    memcpy(RHIInstance.UniformBuffersMapped[_CurrentImage], &Ubo, sizeof(Ubo));
    memcpy(RHIInstance.LightUniformBuffersMapped[_CurrentImage], &Light, sizeof(Light));
}

void UniformBuffer::Cleanup()
{
    for (size_t i = 0; i < RHIInstance.MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroyBuffer(RHIInstance.RHILogicalDevice.device, RHIInstance.UniformBuffers[i], nullptr);
        vkDestroyBuffer(RHIInstance.RHILogicalDevice.device, RHIInstance.LightUniformBuffers[i], nullptr);
        vkFreeMemory(RHIInstance.RHILogicalDevice.device, RHIInstance.UniformBuffersMemory[i], nullptr);
        vkFreeMemory(RHIInstance.RHILogicalDevice.device, RHIInstance.LightUniformBuffersMemory[i], nullptr);
    }
}
