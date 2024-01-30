#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <optional>

#include <set>

#include <cstdint>  //Necessary for uint32_t
#include <limits>  //Necessary for std::numeric_limits
#include <algorithm>  //Necessary for std::clamp
#include <vector>

class RHI;

struct QueueFamilyIndices
{
	std::optional<uint32_t> GraphicsFamily;
	std::optional<uint32_t> PresentFamily;

	bool IsComplete() {
		return GraphicsFamily.has_value() && PresentFamily.has_value();
	}
};

class PhysicalDevice
{
private:

public:

	RHI& RHIInstance;

	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

	VkSampleCountFlagBits MsaaSamples = VK_SAMPLE_COUNT_1_BIT;

	const std::vector<const char*> DeviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	PhysicalDevice(RHI& _RHI);

	void PickPhysicalDevice();

	VkSampleCountFlagBits GetMaxUsableSampleCount();

	bool IsDeviceSuitable(VkPhysicalDevice _Device);

	bool CheckDeviceExtensionSupport(VkPhysicalDevice _Device);

	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice _Device);

	bool HasStencilComponent(VkFormat _Format);

	VkFormat FindDepthFormat();

	VkFormat FindSupportedFormat(const std::vector<VkFormat>& _Candidates, VkImageTiling _Tiling, VkFormatFeatureFlags _Features);

	uint32_t FindMemoryType(uint32_t _TypeFilter, VkMemoryPropertyFlags _Properties);

	void Cleanup();
};