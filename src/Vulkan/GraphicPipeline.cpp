#include "GraphicPipeline.hpp"

#include "RHI.hpp"

GraphicPipeline::GraphicPipeline(RHI& _RHI)
	:RHIInstance(_RHI)
{
}

void GraphicPipeline::CreateGraphicsPipeline()
{
    auto VertShaderCode = RHIInstance.ReadFile("assets/shaders/vert.spv");
    auto FragShaderCode = RHIInstance.ReadFile("assets/shaders/frag.spv");

    VkShaderModule VertShaderModule = RHIInstance.CreateShaderModule(VertShaderCode);
    VkShaderModule FragShaderModule = RHIInstance.CreateShaderModule(FragShaderCode);

    VkPipelineShaderStageCreateInfo VertShaderStageInfo{};
    VertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    VertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    VertShaderStageInfo.module = VertShaderModule;
    VertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo FragShaderStageInfo{};
    FragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    FragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    FragShaderStageInfo.module = FragShaderModule;
    FragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = { VertShaderStageInfo, FragShaderStageInfo };

    VkPipelineVertexInputStateCreateInfo VertexInputInfo{};
    VertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    auto bindingDescription = Vertex::GetBindingDescription();
    auto attributeDescriptions = Vertex::GetAttributeDescriptions();

    VertexInputInfo.vertexBindingDescriptionCount = 1;
    VertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    VertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    VertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo InputAssembly{};
    InputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    InputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    InputAssembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo ViewportState{};
    ViewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    ViewportState.viewportCount = 1;
    ViewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo Rasterizer{};
    Rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    Rasterizer.depthClampEnable = VK_FALSE;
    Rasterizer.rasterizerDiscardEnable = VK_FALSE;
    Rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    Rasterizer.lineWidth = 1.0f;
    Rasterizer.cullMode = VK_CULL_MODE_NONE;
    Rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    Rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo Multisampling{};
    Multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    Multisampling.sampleShadingEnable = VK_TRUE;  //enable sample shading in the pipeline
    Multisampling.minSampleShading = .2f;  //min fraction for sample shading; closer to one is smoother
    Multisampling.rasterizationSamples = RHIInstance.RHIPhysicalDevice.MsaaSamples;

    VkPipelineDepthStencilStateCreateInfo DepthStencil{};
    DepthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    DepthStencil.depthTestEnable = VK_TRUE;
    DepthStencil.depthWriteEnable = VK_TRUE;

    DepthStencil.depthCompareOp = VK_COMPARE_OP_LESS;

    DepthStencil.depthBoundsTestEnable = VK_FALSE;
    DepthStencil.minDepthBounds = 0.0f;  //Optional
    DepthStencil.maxDepthBounds = 1.0f;  //Optional

    DepthStencil.stencilTestEnable = VK_FALSE;
    DepthStencil.front = {};  //Optional
    DepthStencil.back = {};  //Optional

    VkPipelineColorBlendAttachmentState ColorBlendAttachment{};
    ColorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    ColorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo ColorBlending{};
    ColorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    ColorBlending.logicOpEnable = VK_FALSE;
    ColorBlending.logicOp = VK_LOGIC_OP_COPY;
    ColorBlending.attachmentCount = 1;
    ColorBlending.pAttachments = &ColorBlendAttachment;
    ColorBlending.blendConstants[0] = 0.0f;
    ColorBlending.blendConstants[1] = 0.0f;
    ColorBlending.blendConstants[2] = 0.0f;
    ColorBlending.blendConstants[3] = 0.0f;

    std::vector<VkDynamicState> DynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(DynamicStates.size());
    dynamicState.pDynamicStates = DynamicStates.data();

    VkPipelineLayoutCreateInfo PipelineLayoutInfo{};
    PipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    PipelineLayoutInfo.setLayoutCount = 1;
    PipelineLayoutInfo.pSetLayouts = &RHIInstance.DescriptorSetLayout;

    if (vkCreatePipelineLayout(RHIInstance.RHILogicalDevice.device, &PipelineLayoutInfo, nullptr, &RHIInstance.PipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo PipelineInfo{};
    PipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    PipelineInfo.stageCount = 2;
    PipelineInfo.pStages = shaderStages;
    PipelineInfo.pVertexInputState = &VertexInputInfo;
    PipelineInfo.pInputAssemblyState = &InputAssembly;
    PipelineInfo.pViewportState = &ViewportState;
    PipelineInfo.pRasterizationState = &Rasterizer;
    PipelineInfo.pMultisampleState = &Multisampling;
    PipelineInfo.pColorBlendState = &ColorBlending;
    PipelineInfo.pDynamicState = &dynamicState;
    PipelineInfo.layout = RHIInstance.PipelineLayout;
    PipelineInfo.renderPass = RHIInstance.RenderPass;
    PipelineInfo.subpass = 0;
    PipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    PipelineInfo.pDepthStencilState = &DepthStencil;

    if (vkCreateGraphicsPipelines(RHIInstance.RHILogicalDevice.device, VK_NULL_HANDLE, 1, &PipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    vkDestroyShaderModule(RHIInstance.RHILogicalDevice.device, FragShaderModule, nullptr);
    vkDestroyShaderModule(RHIInstance.RHILogicalDevice.device, VertShaderModule, nullptr);
}

void GraphicPipeline::Cleanup()
{
    vkDestroyPipeline(RHIInstance.RHILogicalDevice.device, graphicsPipeline, nullptr);
}
