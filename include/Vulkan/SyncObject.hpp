#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class RHI;

class SyncObject
{
public:

	RHI& RHIInstance;

	SyncObject(RHI& _RHI);

	void CreateSyncObjects();

	void Cleanup();
};