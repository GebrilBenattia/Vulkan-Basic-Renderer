#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

struct PointLight
{
    alignas(16) glm::vec3 Pos;
    alignas(16) glm::vec3 Ambient;
    alignas(16) glm::vec3 Diffuse;
    alignas(16) glm::vec3 Specular;
    alignas(16) glm::vec3 CamPos;
};

class RHI;

class DescriptorSet
{
private:

public:

    RHI& RHIInstance;

    DescriptorSet(RHI& _RHI);

	void CreateDescriptorSets();

	void Cleanup();
};