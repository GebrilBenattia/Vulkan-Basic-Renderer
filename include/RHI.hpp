#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <array>

#include "Window.hpp"
#include "Camera.hpp"
#include "ValidationLayers.hpp"
#include "VulkanInstance.hpp"
#include "Surface.hpp"
#include "PhysicalDevice.hpp"
#include "LogicalDevice.hpp"
#include "Swapchain.hpp"
#include "ImageView.hpp"
#include "GraphicPipeline.hpp"
#include "CommandPool.hpp"
#include "Framebuffer.hpp"
#include "TextureImage.hpp"
#include "VertexBuffer.hpp"
#include "IndexBuffer.hpp"
#include "UniformBuffer.hpp"
#include "DescriptorPool.hpp"
#include "DescriptorSet.hpp"
#include "CommandBuffer.hpp"
#include "SyncObject.hpp"

struct Vertex
{
    glm::vec3 Pos;
    glm::vec3 Color;
    glm::vec3 Normal;
    glm::vec2 TexCoord;

    static VkVertexInputBindingDescription GetBindingDescription()
    {
        VkVertexInputBindingDescription BindingDescription{};
        BindingDescription.binding = 0;
        BindingDescription.stride = sizeof(Vertex);
        BindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return BindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 4> GetAttributeDescriptions()
    {
        std::array<VkVertexInputAttributeDescription, 4> AttributeDescriptions{};

        AttributeDescriptions[0].binding = 0;
        AttributeDescriptions[0].location = 0;
        AttributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        AttributeDescriptions[0].offset = offsetof(Vertex, Pos);

        AttributeDescriptions[1].binding = 0;
        AttributeDescriptions[1].location = 1;
        AttributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        AttributeDescriptions[1].offset = offsetof(Vertex, Color);

        AttributeDescriptions[2].binding = 0;
        AttributeDescriptions[2].location = 2;
        AttributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
        AttributeDescriptions[2].offset = offsetof(Vertex, Normal);

        AttributeDescriptions[3].binding = 0;
        AttributeDescriptions[3].location = 3;
        AttributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
        AttributeDescriptions[3].offset = offsetof(Vertex, TexCoord);

        return AttributeDescriptions;
    }

    bool operator==(const Vertex& _Other) const
    {
        return Pos == _Other.Pos && Color == _Other.Color && TexCoord == _Other.TexCoord;
    }
};

namespace std {
    template<> struct hash<Vertex> {
        size_t operator()(Vertex const& _Vertex) const {
            return ((hash<glm::vec3>()(_Vertex.Pos) ^
                (hash<glm::vec3>()(_Vertex.Color) << 1)) >> 1) ^
                (hash<glm::vec2>()(_Vertex.TexCoord) << 1);
        }
    };
}

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::string MODEL_PATH = "assets/models/viking_room.obj";
const std::string TEXTURE_PATH = "assets/textures/viking_room.png";

#ifdef NDEBUG
    const bool EnableValidationLayers = false;
#else
    const bool EnableValidationLayers = true;
#endif

class RHI
{
public:

    static Camera Cam;

    Window AppWindow;
    VulkanInstance RHIInstance;
    ValidationLayer RHIValidationLayer;
    Surface RHISurface;
    PhysicalDevice RHIPhysicalDevice;
    LogicalDevice RHILogicalDevice;
    Swapchain RHISwapchain;
    ImageView RHIImageView;
    GraphicPipeline RHIGraphicPipeline;
    CommandPool RHICommandPool;
    Framebuffer RHIFramebuffer;
    TextureImage RHITextureImage;
    VertexBuffer RHIVertexBuffer;
    IndexBuffer RHIIndexBuffer;
    UniformBuffer RHIUniformBuffer;
    DescriptorPool RHIDescriptorPool;
    DescriptorSet RHIDescriptorSet;
    CommandBuffer RHICommandBuffer;
    SyncObject RHISyncObject;

    VkQueue GraphicsQueue = nullptr;
    VkQueue PresentQueue = nullptr;
    
    VkRenderPass RenderPass;
    VkDescriptorSetLayout DescriptorSetLayout;
    VkPipelineLayout PipelineLayout;
    
    std::vector<VkCommandBuffer> CommandBuffers;

    std::vector<VkSemaphore> ImageAvailableSemaphores;
    std::vector<VkSemaphore> RenderFinishedSemaphores;
    std::vector<VkFence> InFlightFences;
    uint32_t CurrentFrame = 0;
    

    std::vector<VkBuffer> UniformBuffers;
    std::vector<VkDeviceMemory> UniformBuffersMemory;
    std::vector<void*> UniformBuffersMapped;

    std::vector<VkBuffer> LightUniformBuffers;
    std::vector<VkDeviceMemory> LightUniformBuffersMemory;
    std::vector<void*> LightUniformBuffersMapped;

    std::vector<VkDescriptorSet> DescriptorSets;

    VkImage DepthImage;
    VkDeviceMemory DepthImageMemory;
    VkImageView DepthImageView;

    VkImage ColorImage;
    VkDeviceMemory ColorImageMemory;
    VkImageView ColorImageView;

    bool FramebufferResized = false;

    const int MAX_FRAMES_IN_FLIGHT = 2;

    std::vector<Vertex> Vertices;
    std::vector<uint32_t> Indices;

    RHI();
    ~RHI() {};

    void DrawFrame();

    void CreateColorResources();

    void LoadModel();

    void CreateDepthResources();

    void TransitionImageLayout(VkImage _Image, VkFormat _Format, VkImageLayout _OldLayout, VkImageLayout _NewLayout, uint32_t _MipLevels);

    void CreateImage(uint32_t _Width, uint32_t _Height, uint32_t _MipLevels, VkSampleCountFlagBits _NumSamples, VkFormat _Format, VkImageTiling _Tiling, VkImageUsageFlags _Usage, VkMemoryPropertyFlags _Properties, VkImage& _Image, VkDeviceMemory& _ImageMemory);

    void CreateDescriptorSetLayout();

    static void FramebufferResizeCallback(GLFWwindow* _Window, int _Width, int _Height);

    void CreateRenderPass();

    VkShaderModule CreateShaderModule(const std::vector<char>& _Code);

    static std::vector<char> ReadFile(const std::string& _Filename);

    void Init();

    void Update();

    void Cleanup();
};
