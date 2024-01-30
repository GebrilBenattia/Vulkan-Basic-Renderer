#include "Application.hpp"

float Application::DeltaTime;

Application::Application()
{
}

void Application::processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            RHI::Cam.ProcessKeyboard(FORWARD, 0.016f);

        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            RHI::Cam.ProcessKeyboard(BACKWARD, 0.016f);

        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            RHI::Cam.ProcessKeyboard(LEFT, 0.016f);

        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            RHI::Cam.ProcessKeyboard(RIGHT, 0.016f);

        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            RHI::Cam.ProcessKeyboard(DOWNWARD, 0.016f);

        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            RHI::Cam.ProcessKeyboard(UPWARD, 0.016f);
    }
}

void Application::mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        float xpos = static_cast<float>(xposIn);
        float ypos = static_cast<float>(yposIn);

        if (RHI::Cam.firstMouse)
        {
            RHI::Cam.lastX = xpos;
            RHI::Cam.lastY = ypos;
            RHI::Cam.firstMouse = false;
        }

        float xoffset = xpos - RHI::Cam.lastX;
        float yoffset = RHI::Cam.lastY - ypos; // reversed since y-coordinates go from bottom to top

        RHI::Cam.lastX = xpos;
        RHI::Cam.lastY = ypos;

        RHI::Cam.ProcessMouseMovement(xoffset, yoffset, true);
    }
}

void Application::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        RHI::Cam.ProcessMouseScroll(static_cast<float>(yoffset));
    }
}

void Application::Init()
{
    AppRHI.Init();
}

void Application::Update()
{
    while (!glfwWindowShouldClose(AppRHI.AppWindow.window)) {
        glfwPollEvents();

        processInput(AppRHI.AppWindow.window);
        glfwSetCursorPosCallback(AppRHI.AppWindow.window, Application::mouse_callback);
        glfwSetScrollCallback(AppRHI.AppWindow.window, Application::scroll_callback);
        AppRHI.Cam.Update();

	    AppRHI.Update();
    }
}

void Application::Cleanup()
{
	AppRHI.Cleanup();
}