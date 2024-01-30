#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class Application;

class Window
{
public:

	GLFWwindow* window = nullptr;

	Window();

	void Cleanup();
};

