#pragma once

#include "./VKBase.h"

using namespace Vulkan;
const VkExtent2D& windowSize = GraphicsBase::getBase().getSwapchainCreateInfo().imageExtent;

namespace EasyVulkan {
    using namespace Vulkan;
    struct RenderPassWithFramebuffers {
        RenderPass renderPass;
        std::vector<Framebuffer> framebuffers;
    };
    const auto& createRpwfScreen() {
        static RenderPassWithFramebuffers rpwfScreen;
        if (rpwfScreen.renderPass)
            outStream << std::format("createRpwfScreen() is called more than once.") << std::endl;
        
        VkAttachmentDescription attachmentDescription = {
            .format = GraphicsBase::getBase().getSwapchainCreateInfo().imageFormat,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        };

        VkAttachmentReference attachmentReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
        VkSubpassDescription subpassDescription = {
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .colorAttachmentCount = 1,
            .pColorAttachments = &attachmentReference
        };

        VkSubpassDependency subpassDependency = {
            .srcSubpass = VK_SUBPASS_EXTERNAL,
            .dstSubpass = 0,
            .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask = 0,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
        };

        VkRenderPassCreateInfo renderPassCreateInfo = {
            .attachmentCount = 1,
            .pAttachments = &attachmentDescription,
            .subpassCount = 1,
            .pSubpasses = &subpassDescription,
            .dependencyCount = 1,
            .pDependencies = &subpassDependency
        };
        rpwfScreen.renderPass.create(renderPassCreateInfo);

        rpwfScreen.framebuffers.resize(GraphicsBase::getBase().getSwapchainImageCount());

        VkFramebufferCreateInfo framebufferCreateInfo = {
            .renderPass = rpwfScreen.renderPass,
            .attachmentCount = 1,
            .width = windowSize.width,
            .height = windowSize.height,
            .layers = 1
        };
        
        for (size_t i = 0; i < GraphicsBase::getBase().getSwapchainImageCount(); i++) {
            VkImageView attachment = GraphicsBase::getBase().getSwapchainImageView(i);
            framebufferCreateInfo.pAttachments = &attachment;
            rpwfScreen.framebuffers[i].create(framebufferCreateInfo);
        }

        auto createFramebuffers = [] {
            rpwfScreen.framebuffers.resize(GraphicsBase::getBase().getSwapchainImageCount());
            VkFramebufferCreateInfo framebufferCreateInfo = {
                .renderPass = rpwfScreen.renderPass,
                .attachmentCount = 1,
                .width = windowSize.width,
                .height = windowSize.height,
                .layers = 1
            };
            for (size_t i = 0; i < GraphicsBase::getBase().getSwapchainImageCount(); i++) {
                VkImageView attachment = GraphicsBase::getBase().getSwapchainImageView(i);
                framebufferCreateInfo.pAttachments = &attachment;
                rpwfScreen.framebuffers[i].create(framebufferCreateInfo);
            }
        };
        auto destroyFramebuffers = [] {
            rpwfScreen.framebuffers.clear();
        };
        GraphicsBase::getBase().pushCallbackCreateSwapchain(createFramebuffers);
        GraphicsBase::getBase().pushCallbackDestroySwapchain(destroyFramebuffers);
        createFramebuffers();
        return rpwfScreen;
    }
}