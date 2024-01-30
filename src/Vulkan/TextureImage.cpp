#include "TextureImage.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "RHI.hpp"

TextureImage::TextureImage(RHI& _RHI)
	:RHIInstance(_RHI)
{
}

void TextureImage::CreateTextureImage()
{
    int TexWidth, TexHeight, TexChannels;
    stbi_uc* Pixels = stbi_load(TEXTURE_PATH.c_str(), &TexWidth, &TexHeight, &TexChannels, STBI_rgb_alpha);

    VkDeviceSize ImageSize = (uint64_t)(TexWidth * TexHeight * 4);
    MipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(TexWidth, TexHeight)))) + 1;

    if (!Pixels) {
        throw std::runtime_error("failed to load texture image!");
    }

    VkBuffer StagingBuffer;
    VkDeviceMemory StagingBufferMemory;
    RHIInstance.RHICommandBuffer.CreateBuffer(ImageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, StagingBuffer, StagingBufferMemory);

    void* data;
    vkMapMemory(RHIInstance.RHILogicalDevice.device, StagingBufferMemory, 0, ImageSize, 0, &data);
    memcpy(data, Pixels, static_cast<size_t>(ImageSize));
    vkUnmapMemory(RHIInstance.RHILogicalDevice.device, StagingBufferMemory);

    stbi_image_free(Pixels);

    RHIInstance.CreateImage(TexWidth, TexHeight, MipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, textureImageMemory);

    RHIInstance.TransitionImageLayout(image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, MipLevels);
    RHIInstance.RHICommandBuffer.CopyBufferToImage(StagingBuffer, image, static_cast<uint32_t>(TexWidth), static_cast<uint32_t>(TexHeight));
    //transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmaps

    vkDestroyBuffer(RHIInstance.RHILogicalDevice.device, StagingBuffer, nullptr);
    vkFreeMemory(RHIInstance.RHILogicalDevice.device, StagingBufferMemory, nullptr);

    GenerateMipmaps(image, VK_FORMAT_R8G8B8A8_SRGB, TexWidth, TexHeight, MipLevels);
}

void TextureImage::CreateTextureSampler()
{
    VkSamplerCreateInfo SamplerInfo{};
    SamplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    SamplerInfo.magFilter = VK_FILTER_LINEAR;
    SamplerInfo.minFilter = VK_FILTER_LINEAR;

    SamplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    SamplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    SamplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    VkPhysicalDeviceProperties Properties{};
    vkGetPhysicalDeviceProperties(RHIInstance.RHIPhysicalDevice.physicalDevice, &Properties);

    SamplerInfo.anisotropyEnable = VK_TRUE;
    SamplerInfo.maxAnisotropy = Properties.limits.maxSamplerAnisotropy;

    SamplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

    SamplerInfo.unnormalizedCoordinates = VK_FALSE;

    SamplerInfo.compareEnable = VK_FALSE;
    SamplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

    SamplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    SamplerInfo.minLod = 0.0f;  //Optional
    SamplerInfo.maxLod = static_cast<float>(MipLevels);
    SamplerInfo.mipLodBias = 0.0f;  //Optional

    if (vkCreateSampler(RHIInstance.RHILogicalDevice.device, &SamplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
}

void TextureImage::CreateTextureImageView()
{
    textureImageView = RHIInstance.RHIImageView.CreateImageView(image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, MipLevels);
}

void TextureImage::GenerateMipmaps(VkImage _Image, VkFormat _ImageFormat, int32_t _TexWidth, int32_t _TexHeight, uint32_t _MipLevels)
{
    //Check if image format supports linear blitting
    VkFormatProperties FormatProperties;
    vkGetPhysicalDeviceFormatProperties(RHIInstance.RHIPhysicalDevice.physicalDevice, _ImageFormat, &FormatProperties);

    if (!(FormatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
        throw std::runtime_error("texture image format does not support linear blitting!");
    }

    VkCommandBuffer commandBuffer = RHIInstance.RHICommandBuffer.BeginSingleTimeCommands();

    VkImageMemoryBarrier Barrier{};
    Barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    Barrier.image = image;
    Barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    Barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    Barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    Barrier.subresourceRange.baseArrayLayer = 0;
    Barrier.subresourceRange.layerCount = 1;
    Barrier.subresourceRange.levelCount = 1;

    int32_t MipWidth = _TexWidth;
    int32_t MipHeight = _TexHeight;

    for (uint32_t i = 1; i < MipLevels; i++) {
        Barrier.subresourceRange.baseMipLevel = i - 1;
        Barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        Barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        Barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        Barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
            0, nullptr,
            0, nullptr,
            1, &Barrier);

        VkImageBlit Blit{};
        Blit.srcOffsets[0] = { 0, 0, 0 };
        Blit.srcOffsets[1] = { MipWidth, MipHeight, 1 };
        Blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        Blit.srcSubresource.mipLevel = i - 1;
        Blit.srcSubresource.baseArrayLayer = 0;
        Blit.srcSubresource.layerCount = 1;
        Blit.dstOffsets[0] = { 0, 0, 0 };
        Blit.dstOffsets[1] = { MipWidth > 1 ? MipWidth / 2 : 1, MipHeight > 1 ? MipHeight / 2 : 1, 1 };
        Blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        Blit.dstSubresource.mipLevel = i;
        Blit.dstSubresource.baseArrayLayer = 0;
        Blit.dstSubresource.layerCount = 1;

        vkCmdBlitImage(commandBuffer,
            _Image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            _Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1, &Blit,
            VK_FILTER_LINEAR);

        Barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        Barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        Barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        Barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
            0, nullptr,
            0, nullptr,
            1, &Barrier);

        if (MipWidth > 1) MipWidth /= 2;
        if (MipHeight > 1) MipHeight /= 2;
    }

    Barrier.subresourceRange.baseMipLevel = MipLevels - 1;
    Barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    Barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    Barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    Barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
        0, nullptr,
        0, nullptr,
        1, &Barrier);

    RHIInstance.RHICommandBuffer.EndSingleTimeCommands(commandBuffer);
}

void TextureImage::Cleanup()
{
    vkDestroySampler(RHIInstance.RHILogicalDevice.device, textureSampler, nullptr);
    vkDestroyImageView(RHIInstance.RHILogicalDevice.device, textureImageView, nullptr);

    vkDestroyImage(RHIInstance.RHILogicalDevice.device, image, nullptr);
    vkFreeMemory(RHIInstance.RHILogicalDevice.device, textureImageMemory, nullptr);
}
