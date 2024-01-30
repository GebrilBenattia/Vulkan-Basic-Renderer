#include "RHI.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

Camera RHI::Cam = Camera(0, 0, 2, 0, 1, 0);


void RHI::DrawFrame()
{
    vkWaitForFences(RHILogicalDevice.device, 1, &InFlightFences[CurrentFrame], VK_TRUE, UINT64_MAX);

    uint32_t ImageIndex;
    VkResult Result = vkAcquireNextImageKHR(RHILogicalDevice.device, RHISwapchain.swapChain, UINT64_MAX, ImageAvailableSemaphores[CurrentFrame], VK_NULL_HANDLE, &ImageIndex);

    if (Result == VK_ERROR_OUT_OF_DATE_KHR) {
        RHISwapchain.RecreateSwapChain();
        return;
    }
    else if (Result != VK_SUCCESS && Result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    RHIUniformBuffer.UpdateUniformBuffer(CurrentFrame);

    // Only reset the fence if we are submitting work
    vkResetFences(RHILogicalDevice.device, 1, &InFlightFences[CurrentFrame]);

    vkResetCommandBuffer(CommandBuffers[CurrentFrame], 0);

    RHICommandBuffer.RecordCommandBuffer(CommandBuffers[CurrentFrame], ImageIndex);

    VkSubmitInfo SubmitInfo{};
    SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore WaitSemaphores[] = { ImageAvailableSemaphores[CurrentFrame] };
    VkPipelineStageFlags WaitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    SubmitInfo.waitSemaphoreCount = 1;
    SubmitInfo.pWaitSemaphores = WaitSemaphores;
    SubmitInfo.pWaitDstStageMask = WaitStages;

    SubmitInfo.commandBufferCount = 1;
    SubmitInfo.pCommandBuffers = &CommandBuffers[CurrentFrame];

    VkSemaphore SignalSemaphores[] = { RenderFinishedSemaphores[CurrentFrame] };
    SubmitInfo.signalSemaphoreCount = 1;
    SubmitInfo.pSignalSemaphores = SignalSemaphores;

    if (vkQueueSubmit(GraphicsQueue, 1, &SubmitInfo, InFlightFences[CurrentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR PresentInfo{};
    PresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    PresentInfo.waitSemaphoreCount = 1;
    PresentInfo.pWaitSemaphores = SignalSemaphores;

    VkSwapchainKHR swapChains[] = { RHISwapchain.swapChain };
    PresentInfo.swapchainCount = 1;
    PresentInfo.pSwapchains = swapChains;

    PresentInfo.pImageIndices = &ImageIndex;

    Result = vkQueuePresentKHR(PresentQueue, &PresentInfo);

    if (Result == VK_ERROR_OUT_OF_DATE_KHR || Result == VK_SUBOPTIMAL_KHR || FramebufferResized) {
        FramebufferResized = false;
        RHISwapchain.RecreateSwapChain();
    }
    else if (Result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    CurrentFrame = (CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void RHI::CreateColorResources()
{
    VkFormat ColorFormat = RHISwapchain.SwapChainImageFormat;

    CreateImage(RHISwapchain.SwapChainExtent.width, RHISwapchain.SwapChainExtent.height, 1, RHIPhysicalDevice.MsaaSamples, ColorFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, ColorImage, ColorImageMemory);
    ColorImageView = RHIImageView.CreateImageView(ColorImage, ColorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
}

void RHI::LoadModel()
{
    tinyobj::attrib_t Attrib;
    std::vector<tinyobj::shape_t> Shapes;
    std::vector<tinyobj::material_t> Materials;
    std::string Warn, Err;

    if (!tinyobj::LoadObj(&Attrib, &Shapes, &Materials, &Warn, &Err, MODEL_PATH.c_str())) {
        throw std::runtime_error(Warn + Err);
    }

    std::unordered_map<Vertex, uint32_t> uniqueVertices{};

    for (const auto& shape : Shapes) {
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex{};


            vertex.Pos = {
                Attrib.vertices[3 * index.vertex_index + 0],
                Attrib.vertices[3 * index.vertex_index + 1],
                Attrib.vertices[3 * index.vertex_index + 2]
            };

            vertex.TexCoord = {
                Attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - Attrib.texcoords[2 * index.texcoord_index + 1]
            };

            vertex.Normal = {
                Attrib.normals[3 * index.normal_index + 0],
                Attrib.normals[3 * index.normal_index + 1],
                Attrib.normals[3 * index.normal_index + 2]
            };

            vertex.Color = { 1.0f, 1.0f, 1.0f };

            if (uniqueVertices.count(vertex) == 0) {
                uniqueVertices[vertex] = static_cast<uint32_t>(Vertices.size());
                Vertices.push_back(vertex);
            }

            Indices.push_back(uniqueVertices[vertex]);
        }
    }
}

void RHI::CreateDepthResources()
{
    VkFormat DepthFormat = RHIPhysicalDevice.FindDepthFormat();

    CreateImage(RHISwapchain.SwapChainExtent.width, RHISwapchain.SwapChainExtent.height, 1, RHIPhysicalDevice.MsaaSamples, DepthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, DepthImage, DepthImageMemory);
    DepthImageView = RHIImageView.CreateImageView(DepthImage, DepthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

    TransitionImageLayout(DepthImage, DepthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);
}

void RHI::TransitionImageLayout(VkImage _Image, VkFormat _Format, VkImageLayout _OldLayout, VkImageLayout _NewLayout, uint32_t _MipLevels)
{
    VkCommandBuffer CommandBuffer = RHICommandBuffer.BeginSingleTimeCommands();

    VkImageMemoryBarrier Barrier{};
    Barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    Barrier.oldLayout = _OldLayout;
    Barrier.newLayout = _NewLayout;
    Barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    Barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    Barrier.image = _Image;
    Barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    Barrier.subresourceRange.baseMipLevel = 0;
    Barrier.subresourceRange.levelCount = _MipLevels;
    Barrier.subresourceRange.baseArrayLayer = 0;
    Barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags SourceStage;
    VkPipelineStageFlags DestinationStage;

    if (_NewLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        Barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        if (RHIPhysicalDevice.HasStencilComponent(_Format)) {
            Barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    }
    else {
        Barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    if (_OldLayout == VK_IMAGE_LAYOUT_UNDEFINED && _NewLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        Barrier.srcAccessMask = 0;
        Barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        SourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        DestinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (_OldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && _NewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        Barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        Barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        SourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        DestinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (_OldLayout == VK_IMAGE_LAYOUT_UNDEFINED && _NewLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        Barrier.srcAccessMask = 0;
        Barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        SourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        DestinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }
    else {
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(
        CommandBuffer,
        SourceStage, DestinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &Barrier
    );

    RHICommandBuffer.EndSingleTimeCommands(CommandBuffer);
}

void RHI::CreateImage(uint32_t _Width, uint32_t _Height, uint32_t _MipLevels, VkSampleCountFlagBits _NumSamples, VkFormat _Format, VkImageTiling _Tiling, VkImageUsageFlags _Usage, VkMemoryPropertyFlags _Properties, VkImage& _Image, VkDeviceMemory& _ImageMemory)
{
    VkImageCreateInfo ImageInfo{};
    ImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    ImageInfo.imageType = VK_IMAGE_TYPE_2D;
    ImageInfo.extent.width = _Width;
    ImageInfo.extent.height = _Height;
    ImageInfo.extent.depth = 1;
    ImageInfo.mipLevels = _MipLevels;
    ImageInfo.arrayLayers = 1;
    ImageInfo.format = _Format;
    ImageInfo.tiling = _Tiling;
    ImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ImageInfo.usage = _Usage;
    ImageInfo.samples = _NumSamples;
    ImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(RHILogicalDevice.device, &ImageInfo, nullptr, &_Image) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements MemRequirements;
    vkGetImageMemoryRequirements(RHILogicalDevice.device, _Image, &MemRequirements);

    VkMemoryAllocateInfo AllocInfo{};
    AllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    AllocInfo.allocationSize = MemRequirements.size;
    AllocInfo.memoryTypeIndex = RHIPhysicalDevice.FindMemoryType(MemRequirements.memoryTypeBits, _Properties);

    if (vkAllocateMemory(RHILogicalDevice.device, &AllocInfo, nullptr, &_ImageMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(RHILogicalDevice.device, _Image, _ImageMemory, 0);
}

void RHI::CreateDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding UboLayoutBinding{};
    UboLayoutBinding.binding = 0;
    UboLayoutBinding.descriptorCount = 1;
    UboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    UboLayoutBinding.pImmutableSamplers = nullptr;
    UboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding SamplerLayoutBinding{};
    SamplerLayoutBinding.binding = 1;
    SamplerLayoutBinding.descriptorCount = 1;
    SamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    SamplerLayoutBinding.pImmutableSamplers = nullptr;
    SamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding LightLayoutBinding{};
    LightLayoutBinding.binding = 2;
    LightLayoutBinding.descriptorCount = 1;
    LightLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    LightLayoutBinding.pImmutableSamplers = nullptr;
    LightLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 3> Bindings = { UboLayoutBinding, SamplerLayoutBinding, LightLayoutBinding };
    VkDescriptorSetLayoutCreateInfo LayoutInfo{};
    LayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    LayoutInfo.bindingCount = static_cast<uint32_t>(Bindings.size());
    LayoutInfo.pBindings = Bindings.data();

    if (vkCreateDescriptorSetLayout(RHILogicalDevice.device, &LayoutInfo, nullptr, &DescriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

void RHI::CreateRenderPass()
{
    VkAttachmentDescription ColorAttachment{};
    ColorAttachment.format = RHISwapchain.SwapChainImageFormat;
    ColorAttachment.samples = RHIPhysicalDevice.MsaaSamples;

    ColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    ColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    ColorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    ColorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    ColorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ColorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription ColorAttachmentResolve{};
    ColorAttachmentResolve.format = RHISwapchain.SwapChainImageFormat;
    ColorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
    ColorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    ColorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    ColorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    ColorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    ColorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ColorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference ColorAttachmentRef{};
    ColorAttachmentRef.attachment = 0;
    ColorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription DepthAttachment{};
    DepthAttachment.format = RHIPhysicalDevice.FindDepthFormat();
    DepthAttachment.samples = RHIPhysicalDevice.MsaaSamples;
    DepthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    DepthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    DepthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    DepthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    DepthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    DepthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference DepthAttachmentRef{};
    DepthAttachmentRef.attachment = 1;
    DepthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference ColorAttachmentResolveRef{};
    ColorAttachmentResolveRef.attachment = 2;
    ColorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription Subpass{};
    Subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    Subpass.colorAttachmentCount = 1;
    Subpass.pColorAttachments = &ColorAttachmentRef;
    Subpass.pDepthStencilAttachment = &DepthAttachmentRef;

    Subpass.pResolveAttachments = &ColorAttachmentResolveRef;

    std::array<VkAttachmentDescription, 3> attachments = { ColorAttachment, DepthAttachment, ColorAttachmentResolve };
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &Subpass;

    VkSubpassDependency Dependency{};
    Dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    Dependency.dstSubpass = 0;

    Dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    Dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

    Dependency.srcAccessMask = 0;
    Dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &Dependency;

    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &Dependency;

    if (vkCreateRenderPass(RHILogicalDevice.device, &renderPassInfo, nullptr, &RenderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}

VkShaderModule RHI::CreateShaderModule(const std::vector<char>& _Code)
{
    VkShaderModuleCreateInfo CreateInfo{};
    CreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    CreateInfo.codeSize = _Code.size();
    CreateInfo.pCode = reinterpret_cast<const uint32_t*>(_Code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(RHILogicalDevice.device, &CreateInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
}

std::vector<char> RHI::ReadFile(const std::string& _Filename)
{
    std::ifstream File(_Filename, std::ios::ate | std::ios::binary);

    if (!File.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    size_t FileSize = (size_t)File.tellg();
    std::vector<char> Buffer(FileSize);

    File.seekg(0);
    File.read(Buffer.data(), FileSize);

    File.close();

    return Buffer;
}

RHI::RHI()
    :RHIInstance(VulkanInstance(*this)), RHIValidationLayer(ValidationLayer(*this)), RHISurface(Surface(*this))
    , RHIPhysicalDevice(PhysicalDevice(*this)), RHILogicalDevice(LogicalDevice(*this)), RHISwapchain(Swapchain(*this))
    , RHIImageView(ImageView(*this)), RHIGraphicPipeline(GraphicPipeline(*this)), RHICommandPool(CommandPool(*this))
    , RHIFramebuffer(Framebuffer(*this)), RHITextureImage(TextureImage(*this)), RHIVertexBuffer(VertexBuffer(*this))
    , RHIIndexBuffer(IndexBuffer(*this)), RHIUniformBuffer(UniformBuffer(*this)), RHIDescriptorPool(DescriptorPool(*this))
    , RHIDescriptorSet(DescriptorSet(*this)), RHICommandBuffer(CommandBuffer(*this)), RHISyncObject(SyncObject(*this))
{

}

void RHI::Init()
{
    RHIInstance.CreateInstance();
    RHIValidationLayer.SetupDebugMessenger();
    RHISurface.CreateSurface();
    RHIPhysicalDevice.PickPhysicalDevice();
    RHILogicalDevice.CreateLogicalDevice();
    RHISwapchain.CreateSwapchain();
    RHIImageView.CreateImageViews();

    CreateRenderPass();
    CreateDescriptorSetLayout();
    RHIGraphicPipeline.CreateGraphicsPipeline();

    RHICommandPool.CreateCommandPool();

    CreateColorResources();
    CreateDepthResources();

    RHIFramebuffer.CreateFramebuffers();

    RHITextureImage.CreateTextureImage();
    RHITextureImage.CreateTextureImageView();
    RHITextureImage.CreateTextureSampler();

    LoadModel();

    RHIVertexBuffer.CreateVertexBuffer();
    RHIIndexBuffer.CreateIndexBuffer();
    RHIUniformBuffer.CreateUniformBuffers();
    RHIDescriptorPool.CreateDescriptorPool();
    RHIDescriptorSet.CreateDescriptorSets();
    RHICommandBuffer.CreateCommandBuffers();
    RHISyncObject.CreateSyncObjects();
}

void RHI::Update()
{
    DrawFrame();
}

void RHI::Cleanup()
{
    vkDeviceWaitIdle(RHILogicalDevice.device);

    RHISwapchain.Cleanup();
    RHITextureImage.Cleanup();
    RHIGraphicPipeline.Cleanup();
    
    vkDestroyPipelineLayout(RHILogicalDevice.device, PipelineLayout, nullptr);
    vkDestroyRenderPass(RHILogicalDevice.device, RenderPass, nullptr);

    RHIUniformBuffer.Cleanup();
    RHIDescriptorPool.Cleanup();

    vkDestroyDescriptorSetLayout(RHILogicalDevice.device, DescriptorSetLayout, nullptr);

    RHIIndexBuffer.Cleanup();
    RHIVertexBuffer.Cleanup();
    RHISyncObject.Cleanup();
    RHICommandPool.Cleanup();
    RHILogicalDevice.Cleanup();
    RHIValidationLayer.Cleanup();
    RHISurface.Cleanup();
    RHIInstance.Cleanup();

    AppWindow.Cleanup();
}