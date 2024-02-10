#include "headers/GLFWGeneral.hpp"

using namespace Vulkan;

int main(int argc, char* argv[]) {

    GLFW::initWindow(defaultWindowSize);

    Fence fence(VK_FENCE_CREATE_SIGNALED_BIT);
    Semaphore semaphoreImageIsAvailable;
    Semaphore semaphoreRenderingIsOver;

    CommandBuffer commandBuffer;
    CommandPool commandPool(GraphicsBase::getBase().getQueueFamilyIndexGraphics(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    commandPool.allocateBuffers(commandBuffer);

    while (!GLFW::shouldClose()) {
        GLFW::fps();

        // GraphicsBase::getBase().swapImage(semaphoreImageIsAvailable);
        
        glfwPollEvents();
    }
    
    GLFW::terminateWindow();

    return 0;
}