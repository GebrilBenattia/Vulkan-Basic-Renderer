#include "DescriptorSet.hpp"

#include "RHI.hpp"

DescriptorSet::DescriptorSet(RHI& _RHI)
	:RHIInstance(_RHI)
{
}

void DescriptorSet::CreateDescriptorSets()
{
    std::vector<VkDescriptorSetLayout> layouts(RHIInstance.MAX_FRAMES_IN_FLIGHT, RHIInstance.DescriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = RHIInstance.RHIDescriptorPool.descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(RHIInstance.MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

    RHIInstance.DescriptorSets.resize(RHIInstance.MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(RHIInstance.RHILogicalDevice.device, &allocInfo, RHIInstance.DescriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < RHIInstance.MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = RHIInstance.UniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = RHIInstance.RHITextureImage.textureImageView;
        imageInfo.sampler = RHIInstance.RHITextureImage.textureSampler;

        VkDescriptorBufferInfo lightbufferInfo{};
        lightbufferInfo.buffer = RHIInstance.LightUniformBuffers[i];
        lightbufferInfo.offset = 0;
        lightbufferInfo.range = sizeof(PointLight);

        std::array<VkWriteDescriptorSet, 3> descriptorWrites{};

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = RHIInstance.DescriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = RHIInstance.DescriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &imageInfo;

        descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[2].dstSet = RHIInstance.DescriptorSets[i];
        descriptorWrites[2].dstBinding = 2;
        descriptorWrites[2].dstArrayElement = 0;
        descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[2].descriptorCount = 1;
        descriptorWrites[2].pBufferInfo = &lightbufferInfo;

        vkUpdateDescriptorSets(RHIInstance.RHILogicalDevice.device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
}

void DescriptorSet::Cleanup()
{
}
