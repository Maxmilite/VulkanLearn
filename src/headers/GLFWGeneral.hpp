#pragma once

#include "VKBase.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#pragma comment(lib, "glfw3.lib")

namespace GLFW {

    GLFWwindow* pWindow;
    GLFWmonitor* pMonitor;
    const char* windowTitle = "Vulkan Program";

    void initWindow(VkExtent2D size, bool fullScreen = false, bool isResizable = true, bool limitFrameRate = true) {
        
        if (!glfwInit()) {
            throw std::runtime_error(Message::ERROR_CREATING_WINDOW);
        }
        
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, isResizable);

        pMonitor = glfwGetPrimaryMonitor();

        const GLFWvidmode* pMode = glfwGetVideoMode(pMonitor);

        pWindow = fullScreen ?
            glfwCreateWindow(pMode->width, pMode->height, windowTitle, pMonitor, nullptr) :
            glfwCreateWindow(size.width, size.height, windowTitle, nullptr, nullptr);
        
        if (!pWindow) {
            glfwTerminate();
            throw std::runtime_error(Message::ERROR_CREATING_WINDOW);
        }


    }

    void terminateWindow() {
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