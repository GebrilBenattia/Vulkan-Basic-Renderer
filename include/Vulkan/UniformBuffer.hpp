#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <chrono>


struct UniformBufferObject
{
	glm::mat4 Model;
	glm::mat4 View;
	glm::mat4 Proj;
};

class RHI;

class UniformBuffer
{
private:

public:

	RHI& RHIInstance;

	UniformBuffer(RHI& _RHI);

	void CreateUniformBuffers();

	void UpdateUniformBuffer(uint32_t _CurrentImage);

	void Cleanup();
};