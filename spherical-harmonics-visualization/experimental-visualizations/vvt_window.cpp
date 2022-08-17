#include "vvt_window.hpp"

// std
#include <stdexcept>

namespace vae {
	VvtWindow::VvtWindow(int w, int h, std::string name) : width{ w }, height{ h }, windowName{name}
	{
		initWindow();
	}

	VvtWindow::~VvtWindow() 
	{
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void VvtWindow::initWindow() 
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	}

	void VvtWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface)
	{
		if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create window surface!");
		}
	}

	 void VvtWindow::framebufferResizeCallback(GLFWwindow* window, int width, int height)
	{
		 auto vvtWindow = reinterpret_cast<VvtWindow*>(glfwGetWindowUserPointer(window));
		 vvtWindow->framebufferResized = true;
		 vvtWindow->width = width;
		 vvtWindow->height = height;
	}


}