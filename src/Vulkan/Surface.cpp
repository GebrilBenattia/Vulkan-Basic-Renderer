#include "Surface.hpp"

#include "RHI.hpp"

Surface::Surface(RHI& _RHI)
	:RHIInstance(_RHI)
{
}

void Surface::CreateSurface()
{
    if (glfwCreateWindowSurface(RHIInstance.RHIInstance.instance, RHIInstance.AppWindow.window, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }
}

void Surface::Cleanup()
{
    vkDestroySurfaceKHR(RHIInstance.RHIInstance.instance, surface, nullptr);
}