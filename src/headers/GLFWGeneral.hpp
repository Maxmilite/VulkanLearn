#pragma once

#include "VKBase.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#pragma comment(lib, "glfw3.lib")


// Patch
#include <format>

namespace GLFW {

    GLFWwindow* pWindow;
    GLFWmonitor* pMonitor;
    const char* windowTitle = "Vulkan Program";

    void initWindow(VkExtent2D size, bool fullScreen = false, bool isResizable = true, bool limitFrameRate = false) {
        
        if (!glfwInit()) {
            throw std::runtime_error(Message::ERROR_CREATING_WINDOW);
        }
        
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, isResizable);

        uint32_t extensionCount = 0;
        const char** extensionNames;
        extensionNames = glfwGetRequiredInstanceExtensions(&extensionCount);
        if (!extensionNames) {
            glfwTerminate();
            throw std::runtime_error(Message::ERROR_HARDWARE_NOT_SUPPORT);
        }
        for (size_t i = 0; i < extensionCount; i++)
            Vulkan::GraphicsBase::getBase().pushInstanceExtension(extensionNames[i]);
        Vulkan::GraphicsBase::getBase().pushDeviceExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        pMonitor = glfwGetPrimaryMonitor();

        const GLFWvidmode* pMode = glfwGetVideoMode(pMonitor);

        pWindow = fullScreen ?
            glfwCreateWindow(pMode->width, pMode->height, windowTitle, pMonitor, nullptr) :
            glfwCreateWindow(size.width, size.height, windowTitle, nullptr, nullptr);
        
        if (!pWindow) {
            glfwTerminate();
            throw std::runtime_error(Message::ERROR_CREATING_WINDOW);
        }

        #ifdef _WIN32
            Vulkan::GraphicsBase::getBase().pushInstanceExtension("VK_KHR_surface");
            Vulkan::GraphicsBase::getBase().pushInstanceExtension("VK_KHR_win32_surface");
        #else
            uint32_t extensionCount = 0;
            const char** extensionNames;
            extensionNames = glfwGetRequiredInstanceExtensions(&extensionCount);
            if (!extensionNames) {
                glfwTerminate();
                return false;
                throw std::runtime_error("Vulkan is not available on this machine!");
            }
            for (size_t i = 0; i < extensionCount; i++)
                Vulkan::GraphicsBase::getBase().pushInstanceExtension(extensionNames[i]);
        #endif

            if (Vulkan::GraphicsBase::getBase().createInstance()) throw std::runtime_error("Error while creating instance.");

            VkSurfaceKHR surface = VK_NULL_HANDLE;
            if (VkResult result = glfwCreateWindowSurface(Vulkan::GraphicsBase::getBase().getInstance(), pWindow, nullptr, &surface)) {
                glfwTerminate();
                throw std::runtime_error(std::format("Failed to create a window surface!\nError code: {}\n", int32_t(result)));
            }
            Vulkan::GraphicsBase::getBase().setSurface(surface);

            if (
                Vulkan::GraphicsBase::getBase().getPhysicalDevices() ||
                Vulkan::GraphicsBase::getBase().determinePhysicalDevice(0, true, false) ||
                Vulkan::GraphicsBase::getBase().createDevice())
                throw std::runtime_error("Error while creating devices.");

            if (Vulkan::GraphicsBase::getBase().createSwapchain(limitFrameRate))
                throw std::runtime_error("Error while creating Swapchain.");


    }

    void terminateWindow() {
        Vulkan::GraphicsBase::getBase().waitIdle();
        glfwTerminate();
    }

    bool shouldClose() {
        return glfwWindowShouldClose(pWindow);
    }

    void fps() {
        static double time0 = glfwGetTime();
        static double time1;
        static double dt;
        static int dframe = -1;
        static std::stringstream info;
        time1 = glfwGetTime();
        dframe++;
        if ((dt = time1 - time0) >= 1) {
            info.precision(1);
            info << windowTitle << " | " << std::fixed << dframe / dt << " FPS";
            glfwSetWindowTitle(pWindow, info.str().c_str());
            info.str("");
            time0 = time1;
            dframe = 0;
        }
    }
}