#include "LogicalDevice.hpp"

#include "RHI.hpp"

LogicalDevice::LogicalDevice(RHI& _RHI)
	:RHIInstance(_RHI)
{
}

void LogicalDevice::CreateLogicalDevice() 
{
    QueueFamilyIndices Indices = RHIInstance.RHIPhysicalDevice.FindQueueFamilies(RHIInstance.RHIPhysicalDevice.physicalDevice);

    std::vector<VkDeviceQueueCreateInfo> QueueCreateInfos;
    std::set<uint32_t> UniqueQueueFamilies = { Indices.GraphicsFamily.value(), Indices.PresentFamily.value() };

    float QueuePriority = 1.0f;
    for (uint32_t queueFamily : UniqueQueueFamilies) {
        VkDeviceQueueCreateInfo QueueCreateInfo{};
        QueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        QueueCreateInfo.queueFamilyIndex = queueFamily;
        QueueCreateInfo.queueCount = 1;
        QueueCreateInfo.pQueuePriorities = &QueuePriority;
        QueueCreateInfos.push_back(QueueCreateInfo);
    }

    VkPhysicalDeviceFeatures DeviceFeatures{};
    DeviceFeatures.samplerAnisotropy = VK_TRUE;
    DeviceFeatures.sampleRateShading = VK_TRUE;

    VkDeviceCreateInfo CreateInfo{};
    CreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    CreateInfo.queueCreateInfoCount = static_cast<uint32_t>(QueueCreateInfos.size());
    CreateInfo.pQueueCreateInfos = QueueCreateInfos.data();

    CreateInfo.pEnabledFeatures = &DeviceFeatures;

    CreateInfo.enabledExtensionCount = static_cast<uint32_t>(RHIInstance.RHIPhysicalDevice.DeviceExtensions.size());
    CreateInfo.ppEnabledExtensionNames = RHIInstance.RHIPhysicalDevice.DeviceExtensions.data();

    if (EnableValidationLayers) {
        CreateInfo.enabledLayerCount = static_cast<uint32_t>(RHIInstance.RHIValidationLayer.ValidationLayers.size());
        CreateInfo.ppEnabledLayerNames = RHIInstance.RHIValidationLayer.ValidationLayers.data();
    }
    else {
        CreateInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(RHIInstance.RHIPhysicalDevice.physicalDevice, &CreateInfo, nullptr, &device) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }

    vkGetDeviceQueue(device, Indices.GraphicsFamily.value(), 0, &RHIInstance.GraphicsQueue);
    vkGetDeviceQueue(device, Indices.PresentFamily.value(), 0, &RHIInstance.PresentQueue);
}

void LogicalDevice::Cleanup()
{
	vkDestroyDevice(device, nullptr);
}
