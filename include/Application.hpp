#pragma once

#include "RHI.hpp"

class Application
{
public:

	RHI AppRHI;

	float Time;

	static float DeltaTime;

	Application();

	static void processInput(GLFWwindow* window);
    static void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

	void Init();

	void Update();

	void Cleanup();
};