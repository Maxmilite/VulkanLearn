#include "headers/GLFWGeneral.hpp"
#include "headers/EasyVulkan.hpp"

using namespace Vulkan;

PipelineLayout pipelineLayoutTriangle;
Pipeline pipelineTriangle;
const auto& renderPassAndFramebuffers() {
    static const auto& rpwfScreen = EasyVulkan::createRpwfScreen();
    return rpwfScreen;
}

void createLayout() {
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo {};
    pipelineLayoutTriangle.create(pipelineLayoutCreateInfo);
}

void createPipeline() {
    static ShaderModule vert_triangle("triangle.vert.spv");
    static ShaderModule frag_triangle("triangle.frag.spv");
    static VkPipelineShaderStageCreateInfo shaderStageCreateInfosTriangle[2] = {
        vert_triangle.stageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT),
        frag_triangle.stageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT)
    };
    auto create = [] {
        GraphicsPipelineCreateInfoPack pipelineCiPack;

        pipelineCiPack.createInfo.layout = pipelineLayoutTriangle;
        pipelineCiPack.createInfo.renderPass = renderPassAndFramebuffers().renderPass;

        pipelineCiPack.inputAssemblyStateCi.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;

        pipelineCiPack.viewports.emplace_back(0.f, 0.f, float(windowSize.width), float(windowSize.height), 0.f, 1.f);
        pipelineCiPack.scissors.emplace_back(VkOffset2D {}, windowSize);

        pipelineCiPack.multisampleStateCi.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        pipelineCiPack.colorBlendAttachmentStates.push_back({ .colorWriteMask = 0b1111 });

        pipelineCiPack.updateAllArrays();
        pipelineCiPack.createInfo.stageCount = 2;
        pipelineCiPack.createInfo.pStages = shaderStageCreateInfosTriangle;
        
        pipelineTriangle.create(pipelineCiPack);
    };
    auto destroy = [] {
        pipelineTriangle.~Pipeline();
    };
    GraphicsBase::getBase().pushCallbackCreateSwapchain(create);
    GraphicsBase::getBase().pushCallbackDestroySwapchain(destroy);
    create();
}

int main(int argc, char* argv[]) {

    GLFW::initWindow(defaultWindowSize);

    const auto& [renderPass, framebuffers] = renderPassAndFramebuffers();

    createLayout();
    createPipeline();

    Fence fence(VK_FENCE_CREATE_SIGNALED_BIT);
    Semaphore semaphoreImageIsAvailable;
    Semaphore semaphoreRenderingIsOver;
    Semaphore semaphoreOwnershipIsTransfered;

    CommandBuffer commandBufferGraphics;
    // CommandBuffer commandBufferPresentation;
    CommandPool commandPoolGraphics(GraphicsBase::getBase().getQueueFamilyIndexGraphics(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    // CommandPool commandPoolPresentation(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, GraphicsBase::getBase().getQueueFamilyIndexPresentation());
    commandPoolGraphics.allocateBuffers(commandBufferGraphics);
    // commandPoolPresentation.allocateBuffers(commandBufferPresentation);

    // CommandBuffer commandBuffer;
    // CommandPool commandPool(GraphicsBase::getBase().getQueueFamilyIndexGraphics(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    // commandPool.allocateBuffers(commandBuffer);

    VkClearValue clearColor = { .color = { 0.f, 0.f, 0.f, 1.f } };


    while (!GLFW::shouldClose()) {

        while (glfwGetWindowAttrib(GLFW::pWindow, GLFW_ICONIFIED))
            glfwWaitEvents();

        GLFW::fps();

        GraphicsBase::getBase().swapImage(semaphoreImageIsAvailable);
        
        auto i = GraphicsBase::getBase().getCurrentImageIndex();
        
        fence.waitAndReset();

        commandBufferGraphics.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        // GraphicsBase::getBase().cmdTransferImageOwnership(commandBufferGraphics);
        renderPass.cmdBegin(commandBufferGraphics, framebuffers[i], { {}, windowSize }, clearColor);

        vkCmdBindPipeline(commandBufferGraphics, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineTriangle);
        vkCmdDraw(commandBufferGraphics, 3, 1, 0, 0);

        renderPass.cmdEnd(commandBufferGraphics);

        commandBufferGraphics.end();
        GraphicsBase::getBase().submitCommandBufferGraphics(commandBufferGraphics, semaphoreImageIsAvailable, semaphoreRenderingIsOver, fence);

        // commandBufferPresentation.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        // GraphicsBase::getBase().cmdTransferImageOwnership(commandBufferPresentation);
        // commandBufferPresentation.end();
        // GraphicsBase::getBase().submitCommandBufferPresentation(commandBufferPresentation, VK_NULL_HANDLE, semaphoreOwnershipIsTransfered, fence);

        GraphicsBase::getBase().presentImage(semaphoreRenderingIsOver);

        glfwPollEvents();
    }
    
    GLFW::terminateWindow();

    return 0;
}