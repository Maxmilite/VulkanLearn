#include "headers/GLFWGeneral.hpp"
#include "headers/EasyVulkan.hpp"

using namespace Vulkan;

int main(int argc, char* argv[]) {

    GLFW::initWindow(defaultWindowSize);

    const auto& [renderPass, framebuffers] = EasyVulkan::createRpwfScreen();

    Fence fence(VK_FENCE_CREATE_SIGNALED_BIT);
    Semaphore semaphoreImageIsAvailable;
    Semaphore semaphoreRenderingIsOver;
    Semaphore semaphoreOwnershipIsTransfered;

    CommandBuffer commandBufferGraphics;
    CommandBuffer commandBufferPresentation;
    CommandPool commandPoolGraphics(GraphicsBase::getBase().getQueueFamilyIndexGraphics(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    CommandPool commandPoolPresentation(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, GraphicsBase::getBase().getQueueFamilyIndexPresentation());
    commandPoolGraphics.allocateBuffers(commandBufferGraphics);
    commandPoolPresentation.allocateBuffers(commandBufferPresentation);

    // CommandBuffer commandBuffer;
    // CommandPool commandPool(GraphicsBase::getBase().getQueueFamilyIndexGraphics(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    // commandPool.allocateBuffers(commandBuffer);

    VkClearValue clearColor = { .color = { 1.f, 0.f, 0.f, 1.f } };

    while (!GLFW::shouldClose()) {

        while (glfwGetWindowAttrib(GLFW::pWindow, GLFW_ICONIFIED))
            glfwWaitEvents();

        GLFW::fps();

        GraphicsBase::getBase().swapImage(semaphoreImageIsAvailable);
        
        auto i = GraphicsBase::getBase().getCurrentImageIndex();
        
        fence.waitAndReset();

        commandBufferGraphics.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        GraphicsBase::getBase().cmdTransferImageOwnership(commandBufferGraphics);
        renderPass.cmdBegin(commandBufferGraphics, framebuffers[i], { {}, windowSize }, clearColor);
        renderPass.cmdEnd(commandBufferGraphics);

        commandBufferGraphics.end();
        GraphicsBase::getBase().submitCommandBufferGraphics(commandBufferGraphics, semaphoreImageIsAvailable);

        commandBufferPresentation.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        GraphicsBase::getBase().cmdTransferImageOwnership(commandBufferPresentation);
        commandBufferPresentation.end();
        GraphicsBase::getBase().submitCommandBufferPresentation(commandBufferPresentation, VK_NULL_HANDLE, semaphoreOwnershipIsTransfered, fence);

        GraphicsBase::getBase().presentImage(semaphoreOwnershipIsTransfered);

        glfwPollEvents();
    }
    
    GLFW::terminateWindow();

    return 0;
}