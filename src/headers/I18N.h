#pragma once

#include <string>

namespace Message {
    const std::string ERROR_CREATING_WINDOW = "Error while creating vulkan window.";
    const std::string ERROR_HARDWARE_NOT_SUPPORT = "Vulkan is not supported by hardware.";
    const std::string ERROR_CREATING_INSTANCE = "Failed to create a vulkan instance!";
    const std::string ERROR_COUNTING_INSTANCE = "Failed to get the count of instance layers!";
    const std::string ERROR_POINTER_MESSENGER = "Failed to get the function pointer of vkCreateDebugUtilsMessengerEXT!";
    const std::string ERROR_CREATING_DEBUG_MESSENGER = "Failed to create a debug messenger!";
    const std::string ERROR_CREATING_SURFACE = "Failed to create a window surface!";
    const std::string ERROR_COUNTING_EXT = "Failed to get the count of instance extensions!";

    // Submitted. All messages following will not be included here.
}