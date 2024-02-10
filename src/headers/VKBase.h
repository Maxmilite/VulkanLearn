#pragma once

#include "./VKUtil.h"
#include <vulkan/vulkan_core.h>

namespace Vulkan {

    constexpr VkExtent2D defaultWindowSize = { 1600, 900 };

    class GraphicsBase {

        static GraphicsBase singleton;
        GraphicsBase() = default;
        GraphicsBase(GraphicsBase&&) = delete;
        ~GraphicsBase() {
            if (!instance)
                return;
            if (device) {
                waitIdle();
                if (swapchain) {
                    for (auto& i : callbacksDestroySwapchain) i();
                    for (auto& i : swapchainImageViews)
                        if (i)
                            vkDestroyImageView(device, i, nullptr);
                    vkDestroySwapchainKHR(device, swapchain, nullptr);
                }
                // for (auto& i : callbacksDestroyDevice) i();
                vkDestroyDevice(device, nullptr);
            }
            if (surface)
                vkDestroySurfaceKHR(instance, surface, nullptr);
            if (debugUtilsMessenger) {
                PFN_vkDestroyDebugUtilsMessengerEXT DestroyDebugUtilsMessenger =
                    reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
                if (DestroyDebugUtilsMessenger)
                    DestroyDebugUtilsMessenger(instance, debugUtilsMessenger, nullptr);
            }
            vkDestroyInstance(instance, nullptr);
        }
    
    public:
        static GraphicsBase& getBase() {
            return singleton;
        }

    // Layers & Extensions
    private:
        VkInstance instance;
        std::vector<const char*> instanceLayers;
        std::vector<const char*> instanceExtensions;

        static void addLayerOrExtension(std::vector<const char*>& container, const char* name) {
            for (auto& i : container) if (!strcmp(name, i)) return;
            container.push_back(name);
        }

    public:
        VkInstance getInstance() const { return instance; }
        const std::vector<const char*>& getInstanceLayers() const { return instanceLayers; }
        const std::vector<const char*>& getInstanceExtensions() const { return instanceExtensions; }

        void pushInstanceLayer(const char* layerName) {
            addLayerOrExtension(instanceLayers, layerName);
        }

        void pushInstanceExtension(const char* extensionName) {
            addLayerOrExtension(instanceExtensions, extensionName);
        }
        
        VkResult createInstance(const void* pNext = nullptr, VkInstanceCreateFlags flags = 0) {
        #ifndef NDEBUG
            pushInstanceLayer("VK_LAYER_KHRONOS_validation");
            pushInstanceExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        #endif
            VkApplicationInfo applicatianInfo = {
                .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                .apiVersion = VK_API_VERSION_1_3
            }; 
            VkInstanceCreateInfo instanceCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                .pNext = pNext,
                .flags = flags,
                .pApplicationInfo = &applicatianInfo,
                .enabledLayerCount = uint32_t(instanceLayers.size()),
                .ppEnabledLayerNames = instanceLayers.data(),
                .enabledExtensionCount = uint32_t(instanceExtensions.size()),
                .ppEnabledExtensionNames = instanceExtensions.data()
            };

            if (VkResult result = vkCreateInstance(&instanceCreateInfo, nullptr, &instance)) {
                std::cerr << Message::ERROR_CREATING_INSTANCE << std::endl;
                return result;
            }

            std::cout << std::format(
                    "Vulkan API Version: {}.{}.{}\n",
                    VK_VERSION_MAJOR(VK_API_VERSION_1_3),
                    VK_VERSION_MINOR(VK_API_VERSION_1_3),
                    VK_VERSION_PATCH(VK_API_VERSION_1_3));
        #ifndef NDEBUG
            createDebugMessenger();
        #endif
            return VK_SUCCESS;
        }
        
        VkResult checkInstanceLayers(std::span<const char*> layersToCheck) {
            uint32_t layerCount = 0;
            std::vector<VkLayerProperties> availableLayers;
            if (VkResult result = vkEnumerateInstanceLayerProperties(&layerCount, nullptr)) {
                std::cerr << Message::ERROR_COUNTING_INSTANCE << std::endl;
                return result;
            }
            if (layerCount) {
                availableLayers.resize(layerCount);
                if (VkResult result = vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data())) {
                    std::cerr << Message::ERROR_COUNTING_INSTANCE << std::endl;
                    return result;
                }
                for (auto& i : layersToCheck) {
                    bool found = false;
                    for (auto& j : availableLayers)
                        if (!strcmp(i, j.layerName)) {
                            found = true;
                            break;
                        }
                    if (!found) i = nullptr;
                }
            }
            else for (auto& i : layersToCheck) i = nullptr;
            return VK_SUCCESS;
        }

        void setInstanceLayers(const std::vector<const char*>& layerNames) {
            instanceLayers = layerNames;
        }

        VkResult checkInstanceExtensions(std::span<const char*> extensionsToCheck, const char* layerName = nullptr) const {
            uint32_t extensionCount;
            std::vector<VkExtensionProperties> availableExtensions;
            if (VkResult result = vkEnumerateInstanceExtensionProperties(layerName, &extensionCount, nullptr)) {
                layerName ? 
                std::cerr << Message::ERROR_COUNTING_EXT << "\n Layer Name: " << layerName << std::endl : 
                std::cerr << Message::ERROR_COUNTING_EXT << "\n Layer Name: " << layerName << std::endl;
                return result;
            }
            if (extensionCount) {
                availableExtensions.resize(extensionCount);
                if (VkResult result = vkEnumerateInstanceExtensionProperties(layerName, &extensionCount, availableExtensions.data())) {
                    std::cerr << Message::ERROR_COUNTING_EXT << std::endl;
                    return result;
                }
                for (auto& i : extensionsToCheck) {
                    bool found = false;
                    for (auto& j : availableExtensions)
                        if (!strcmp(i, j.extensionName)) {
                            found = true;
                            break;
                        }
                    if (!found)
                        i = nullptr;
                }
            }
            else
                for (auto& i : extensionsToCheck)
                    i = nullptr;
            return VK_SUCCESS;
        }

        void setInstanceExtensions(const std::vector<const char*>& extensionNames) {
            instanceExtensions = extensionNames;
        }
    
    // Debug Messenger
    private:
        VkDebugUtilsMessengerEXT debugUtilsMessenger;
        VkResult createDebugMessenger() {
            static PFN_vkDebugUtilsMessengerCallbackEXT DebugUtilsMessengerCallback = [] (
                VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                void* pUserData
            ) -> VkBool32 {
                std::cerr << std::format("{}\n\n", pCallbackData->pMessage);
                return VK_FALSE;
            };
            VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
                .messageSeverity =
                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
                .messageType =
                    VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                    VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                    VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
                .pfnUserCallback = DebugUtilsMessengerCallback
            };

            PFN_vkCreateDebugUtilsMessengerEXT createDebugUtilsMessenger =
                reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
                    vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT")
                );
                
            if (createDebugUtilsMessenger) {
                VkResult result = createDebugUtilsMessenger(instance, &debugUtilsMessengerCreateInfo, nullptr, &debugUtilsMessenger);
                if (result) std::cerr << Message::ERROR_CREATING_DEBUG_MESSENGER << std::endl;
                return result;
            }
            std::cerr << Message::ERROR_POINTER_MESSENGER << std::endl;
            return VK_RESULT_MAX_ENUM;
        }

    
    // Surface
    private:
        VkSurfaceKHR surface;

    public:
        VkSurfaceKHR getSurface() const {
            return surface;
        }

    void setSurface(VkSurfaceKHR surface) {
        if (!this->surface)
            this->surface = surface;
    }

    // Device
    private:
        VkPhysicalDevice physicalDevice;
        VkPhysicalDeviceProperties physicalDeviceProperties;
        VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
        std::vector<VkPhysicalDevice> availablePhysicalDevices;

        VkDevice device;
        uint32_t queueFamilyIndexGraphics = VK_QUEUE_FAMILY_IGNORED;
        uint32_t queueFamilyIndexPresentation = VK_QUEUE_FAMILY_IGNORED;
        uint32_t queueFamilyIndexCompute = VK_QUEUE_FAMILY_IGNORED;
        VkQueue queueGraphics;
        VkQueue queuePresentation;
        VkQueue queueCompute;

        std::vector<const char*> deviceExtensions;

        VkResult getQueueFamilyIndices(VkPhysicalDevice physicalDevice, bool enableGraphicsQueue, bool enableComputeQueue, uint32_t (&queueFamilyIndices)[3]) {
            uint32_t queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
            if (!queueFamilyCount)
                return VK_RESULT_MAX_ENUM;
            std::vector<VkQueueFamilyProperties> queueFamilyPropertieses(queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyPropertieses.data());
            auto& [ig, ip, ic] = queueFamilyIndices;
            ig = ip = ic = VK_QUEUE_FAMILY_IGNORED;
            for (uint32_t i = 0; i < queueFamilyCount; i++) {
                VkBool32
                    supportGraphics = enableGraphicsQueue && queueFamilyPropertieses[i].queueFlags & VK_QUEUE_GRAPHICS_BIT,
                    supportPresentation = false,
                    supportCompute = enableComputeQueue && queueFamilyPropertieses[i].queueFlags & VK_QUEUE_COMPUTE_BIT;
                if (surface)
                    if (VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &supportPresentation)) {
                        std::cerr << "Failed to determine if the queue family supports presentation!" << std::endl;
                        return result;
                    }
                if (supportGraphics && supportCompute) {
                    if (supportPresentation) {
                        ig = ip = ic = i;
                        break;
                    }
                    if (ig != ic || ig == VK_QUEUE_FAMILY_IGNORED)
                        ig = ic = i;
                    if (!surface)
                        break;
                }
                if (supportGraphics && ig == VK_QUEUE_FAMILY_IGNORED)
                    ig = i;
                if (supportPresentation && ip == VK_QUEUE_FAMILY_IGNORED)
                    ip = i;
                if (supportCompute && ic == VK_QUEUE_FAMILY_IGNORED)
                    ic = i;
            }
            if (ig == VK_QUEUE_FAMILY_IGNORED && enableGraphicsQueue ||
                ip == VK_QUEUE_FAMILY_IGNORED && surface ||
                ic == VK_QUEUE_FAMILY_IGNORED && enableComputeQueue)
                return VK_RESULT_MAX_ENUM;
            queueFamilyIndexGraphics = ig;
            queueFamilyIndexPresentation = ip;
            queueFamilyIndexCompute = ic;
            return VK_SUCCESS;
        }

    public:
        VkPhysicalDevice getPhysicalDevice() const {
            return physicalDevice;
        }
        const VkPhysicalDeviceProperties& getPhysicalDeviceProperties() const {
            return physicalDeviceProperties;
        }
        const VkPhysicalDeviceMemoryProperties& getPhysicalDeviceMemoryProperties() const {
            return physicalDeviceMemoryProperties;
        }
        VkPhysicalDevice getAvailablePhysicalDevice(uint32_t index) const {
            return availablePhysicalDevices[index];
        }
        uint32_t getAvailablePhysicalDeviceCount() const {
            return uint32_t(availablePhysicalDevices.size());
        }

        VkDevice getDevice() const {
            return device;
        }
        uint32_t getQueueFamilyIndexGraphics() const {
            return queueFamilyIndexGraphics;
        }
        uint32_t getQueueFamilyIndexPresentation() const {
            return queueFamilyIndexPresentation;
        }
        uint32_t getQueueFamilyIndexCompute() const {
            return queueFamilyIndexCompute;
        }
        VkQueue getQueueGraphics() const {
            return queueGraphics;
        }
        VkQueue getQueuePresentation() const {
            return queuePresentation;
        }
        VkQueue getQueueCompute() const {
            return queueCompute;
        }

        const std::vector<const char*>& getDeviceExtensions() const {
            return deviceExtensions;
        }

        void pushDeviceExtension(const char* extensionName) {
            addLayerOrExtension(deviceExtensions, extensionName);
        }

        VkResult getPhysicalDevices() {
            uint32_t deviceCount;
            if (VkResult result = vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr)) {
                std::cerr << std::format("Failed to get the count of physical devices!\nError code: {}\n", int32_t(result));
                return result;
            }
            if (!deviceCount)
                throw std::runtime_error(std::format("Failed to find any physical device supports vulkan!\n"));
            availablePhysicalDevices.resize(deviceCount);
            VkResult result = vkEnumeratePhysicalDevices(instance, &deviceCount, availablePhysicalDevices.data());
            if (result)
                std::cerr << std::format("Failed to enumerate physical devices!\nError code: {}\n", int32_t(result));
            return result;
        }

        VkResult determinePhysicalDevice(uint32_t deviceIndex = 0, bool enableGraphicsQueue = true, bool enableComputeQueue = true) {
            static constexpr uint32_t notFound = INT32_MAX;
            struct queueFamilyIndexCombination {
                uint32_t graphics = VK_QUEUE_FAMILY_IGNORED;
                uint32_t presentation = VK_QUEUE_FAMILY_IGNORED;
                uint32_t compute = VK_QUEUE_FAMILY_IGNORED;
            };
            static std::vector<queueFamilyIndexCombination> queueFamilyIndexCombinations(availablePhysicalDevices.size());
            auto& [ig, ip, ic] = queueFamilyIndexCombinations[deviceIndex];

            if (ig == notFound && enableGraphicsQueue ||
                ip == notFound && surface ||
                ic == notFound && enableComputeQueue)
                return VK_RESULT_MAX_ENUM;

            if (ig == VK_QUEUE_FAMILY_IGNORED && enableGraphicsQueue ||
                ip == VK_QUEUE_FAMILY_IGNORED && surface ||
                ic == VK_QUEUE_FAMILY_IGNORED && enableComputeQueue) {
                uint32_t indices[3];
                VkResult result = getQueueFamilyIndices(availablePhysicalDevices[deviceIndex], enableGraphicsQueue, enableComputeQueue, indices);
                if (result == VK_SUCCESS ||
                    result == VK_RESULT_MAX_ENUM) {
                    if (enableGraphicsQueue)
                        ig = indices[0] & INT32_MAX;
                    if (surface)
                        ip = indices[1] & INT32_MAX;
                    if (enableComputeQueue)
                        ic = indices[2] & INT32_MAX;
                }
                if (result)
                    return result;
            } else {
                queueFamilyIndexGraphics = enableGraphicsQueue ? ig : VK_QUEUE_FAMILY_IGNORED;
                queueFamilyIndexPresentation = surface ? ip : VK_QUEUE_FAMILY_IGNORED;
                queueFamilyIndexCompute = enableComputeQueue ? ic : VK_QUEUE_FAMILY_IGNORED;
            }
            physicalDevice = availablePhysicalDevices[deviceIndex];
            return VK_SUCCESS;
        }

        VkResult createDevice(const void* pNext = nullptr, VkDeviceCreateFlags flags = 0) {
            float queuePriority = 1.f;
            VkDeviceQueueCreateInfo queueCreateInfos[3] = {
                {
                    .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                    .queueCount = 1,
                    .pQueuePriorities = &queuePriority },
                {
                    .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                    .queueCount = 1,
                    .pQueuePriorities = &queuePriority },
                {
                    .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                    .queueCount = 1,
                    .pQueuePriorities = &queuePriority } };
            uint32_t queueCreateInfoCount = 0;
            if (queueFamilyIndexGraphics != VK_QUEUE_FAMILY_IGNORED)
                queueCreateInfos[queueCreateInfoCount++].queueFamilyIndex = queueFamilyIndexGraphics;
            if (queueFamilyIndexPresentation != VK_QUEUE_FAMILY_IGNORED &&
                queueFamilyIndexPresentation != queueFamilyIndexGraphics)
                queueCreateInfos[queueCreateInfoCount++].queueFamilyIndex = queueFamilyIndexPresentation;
            if (queueFamilyIndexCompute != VK_QUEUE_FAMILY_IGNORED &&
                queueFamilyIndexCompute != queueFamilyIndexGraphics &&
                queueFamilyIndexCompute != queueFamilyIndexPresentation)
                queueCreateInfos[queueCreateInfoCount++].queueFamilyIndex = queueFamilyIndexCompute;

            VkPhysicalDeviceFeatures physicalDeviceFeatures;
            vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatures);

            VkDeviceCreateInfo deviceCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                .pNext = pNext,
                .flags = flags,
                .queueCreateInfoCount = queueCreateInfoCount,
                .pQueueCreateInfos = queueCreateInfos,
                .enabledExtensionCount = uint32_t(deviceExtensions.size()),
                .ppEnabledExtensionNames = deviceExtensions.data(),
                .pEnabledFeatures = &physicalDeviceFeatures
            };

            if (VkResult result = vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device)) {
                std::cerr << std::format("Failed to create a vulkan logical device!\nError code: {}", int32_t(result)) << std::endl;
                return result;
            }

            if (queueFamilyIndexGraphics != VK_QUEUE_FAMILY_IGNORED)
                vkGetDeviceQueue(device, queueFamilyIndexGraphics, 0, &queueGraphics);
            if (queueFamilyIndexPresentation != VK_QUEUE_FAMILY_IGNORED)
                vkGetDeviceQueue(device, queueFamilyIndexPresentation, 0, &queuePresentation);
            if (queueFamilyIndexCompute != VK_QUEUE_FAMILY_IGNORED)
                vkGetDeviceQueue(device, queueFamilyIndexCompute, 0, &queueCompute);
        
            vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
            vkGetPhysicalDeviceMemoryProperties(physicalDevice, &physicalDeviceMemoryProperties);
            std::cout << std::format("Renderer: {}", physicalDeviceProperties.deviceName) << std::endl;
            
            return VK_SUCCESS;
        }

        VkResult checkDeviceExtensions(std::span<const char*> extensionsToCheck, const char* layerName = nullptr) const {
            return VK_SUCCESS;
        }

        void getDeviceExtensions(const std::vector<const char*>& extensionNames) {
            deviceExtensions = extensionNames;
        }

    // Image View
    private:
        std::vector <VkSurfaceFormatKHR> availableSurfaceFormats;

        VkSwapchainKHR swapchain;
        std::vector <VkImage> swapchainImages;
        std::vector <VkImageView> swapchainImageViews;
        VkSwapchainCreateInfoKHR swapchainCreateInfo = {};

        VkResult createSwapchainInternal() {

            if (VkResult result = vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &swapchain)) {
                std::cerr << std::format("Failed to create a swapchain.\nError code: {}", int32_t(result)) << std::endl;
                return result;
            }

            uint32_t swapchainImageCount;
            if (VkResult result = vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, nullptr)) {
                std::cerr << std::format("Failed to get the count of swapchain images.\nError code: {}", int32_t(result)) << std::endl;
                return result;
            }
            swapchainImages.resize(swapchainImageCount);
            if (VkResult result = vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, swapchainImages.data())) {
                std::cerr << std::format("Failed to get swapchain images.\nError code: {}", int32_t(result)) << std::endl;
                return result;
            }

            swapchainImageViews.resize(swapchainImageCount);
            VkImageViewCreateInfo imageViewCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = swapchainCreateInfo.imageFormat,
                .subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
            };
            for (size_t i = 0; i < swapchainImageCount; i++) {
                imageViewCreateInfo.image = swapchainImages[i];
                if (VkResult result = vkCreateImageView(device, &imageViewCreateInfo, nullptr, &swapchainImageViews[i])) {
                    std::cerr << std::format("Failed to create a swapchain image view.\nError code: {}\n", int32_t(result)) << std::endl;
                    return result;
                }
            }

            return VK_SUCCESS;
        }

    public:
        const VkFormat& getAvailableSurfaceFormat(uint32_t index) const {
            return availableSurfaceFormats[index].format;
        }

        const VkColorSpaceKHR& getAvailableSurfaceColorSpace(uint32_t index) const {
            return availableSurfaceFormats[index].colorSpace;
        }

        uint32_t getAvailableSurfaceFormatCount() const {
            return uint32_t(availableSurfaceFormats.size());
        }

        VkSwapchainKHR getSwapchain() const {
            return swapchain;
        }

        VkImage getSwapchainImage(uint32_t index) const {
            return swapchainImages[index];
        }

        VkImageView getSwapchainImageView(uint32_t index) const {
            return swapchainImageViews[index];
        }

        uint32_t getSwapchainImageCount() const {
            return uint32_t(swapchainImages.size());
        }

        const VkSwapchainCreateInfoKHR& getSwapchainCreateInfo() const {
            return swapchainCreateInfo;
        }

        VkResult getSurfaceFormats() {
            uint32_t surfaceFormatCount;
            if (VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, nullptr)) {
                std::cerr << std::format("Failed to get the count of surface formats!\nError code: {}", int32_t(result)) << std::endl;
                return result;
            }
            if (!surfaceFormatCount)
                throw std::runtime_error("Failed to find any supported surface format!");
            availableSurfaceFormats.resize(surfaceFormatCount);
            VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, availableSurfaceFormats.data());
            if (result)
                std::cerr << std::format("Failed to get surface formats!\nError code: {}", int32_t(result)) << std::endl;
            
            
            return result;
        }

        VkResult setSurfaceFormat(VkSurfaceFormatKHR surfaceFormat) {
            bool formatIsAvailable = false;
            if (!surfaceFormat.format) {
                for (auto& i : availableSurfaceFormats)
                    if (i.colorSpace == surfaceFormat.colorSpace) {
                        swapchainCreateInfo.imageFormat = i.format;
                        swapchainCreateInfo.imageColorSpace = i.colorSpace;
                        formatIsAvailable = true;
                        break;
                    }
            } else {
                for (auto& i : availableSurfaceFormats)
                    if (i.format == surfaceFormat.format &&
                        i.colorSpace == surfaceFormat.colorSpace) {
                        swapchainCreateInfo.imageFormat = i.format;
                        swapchainCreateInfo.imageColorSpace = i.colorSpace;
                        formatIsAvailable = true;
                        break;
                    }
            }
            if (!formatIsAvailable) return VK_ERROR_FORMAT_NOT_SUPPORTED;
            if (swapchain) return recreateSwapchain();
            return VK_SUCCESS;
        }

        VkResult createSwapchain(bool limitFrameRate = false, const void* pNext = nullptr, VkSwapchainCreateFlagsKHR flags = 0) {
            VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
            if (VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities)) {
                std::cerr << std::format("Failed to get physical device surface capabilities!\nError code: {}", int32_t(result)) << std::endl;
                return result;
            }

            swapchainCreateInfo.minImageCount = surfaceCapabilities.minImageCount + (surfaceCapabilities.maxImageCount > surfaceCapabilities.minImageCount);
        
            swapchainCreateInfo.imageExtent =
                surfaceCapabilities.currentExtent.width == -1 ?
                VkExtent2D{
                    glm::clamp(defaultWindowSize.width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width),
                    glm::clamp(defaultWindowSize.height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height) } :
                surfaceCapabilities.currentExtent;

            swapchainCreateInfo.preTransform = surfaceCapabilities.currentTransform;

            if (surfaceCapabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR)
                swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
            else for (size_t i = 0; i < 4; i++)
                if (surfaceCapabilities.supportedCompositeAlpha & 1 << i) {
                    swapchainCreateInfo.compositeAlpha = VkCompositeAlphaFlagBitsKHR(
                        surfaceCapabilities.supportedCompositeAlpha & 1 << i
                    );
                    break;
                }
            
            swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            if (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
                swapchainCreateInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
            if (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
                swapchainCreateInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
            else
                std::cerr << std::format("VK_IMAGE_USAGE_TRANSFER_DST_BIT isn't supported.") << std::endl;

            if (availableSurfaceFormats.empty())
                if (VkResult result = getSurfaceFormats()) return result;
            

            if (!swapchainCreateInfo.imageFormat) {
                if (setSurfaceFormat({ VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR }) &&
                    setSurfaceFormat({ VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })) {
                    swapchainCreateInfo.imageFormat = availableSurfaceFormats[0].format;
                    swapchainCreateInfo.imageColorSpace = availableSurfaceFormats[0].colorSpace;
                    std::cerr << std::format("Failed to select a four-component UNORM surface format.") << std::endl;
                }
            }

            uint32_t surfacePresentModeCount;
            if (VkResult result = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &surfacePresentModeCount, nullptr)) {
                std::cerr << std::format("Failed to get the count of surface present modes.\nError code: {}", int32_t(result)) << std::endl;
                return result;
            }
            if (!surfacePresentModeCount)
                throw std::runtime_error("Failed to find any surface present mode.");

            std::vector<VkPresentModeKHR> surfacePresentModes(surfacePresentModeCount);
            if (VkResult result = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &surfacePresentModeCount, surfacePresentModes.data())) {
                std::cerr << std::format("Failed to get surface present modes.\nError code: {}", int32_t(result)) << std::endl;
                return result;
            }

            swapchainCreateInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
            if (!limitFrameRate)
                for (size_t i = 0; i < surfacePresentModeCount; i++)
                    if (surfacePresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
                        swapchainCreateInfo.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
                        break;
                    }

            swapchainCreateInfo.imageArrayLayers = 1;
            swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
            swapchainCreateInfo.pNext = pNext;
            swapchainCreateInfo.flags = flags;
            swapchainCreateInfo.surface = surface;
            swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            swapchainCreateInfo.clipped = VK_TRUE;

            if (VkResult result = vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &swapchain)) {
                std::cerr << std::format("Failed to create a swapchain.\nError code: {}", int32_t(result)) << std::endl;
                return result;
            }

            if (VkResult result = createSwapchainInternal())
                return result;
            for (auto& i : callbacksCreateSwapchain) i();
            return VK_SUCCESS;
        }
        
        VkResult recreateSwapchain() {
            VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
            if (VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities)) {
                std::cerr << std::format("Failed to get physical device surface capabilities.\nError code: {}", int32_t(result)) << std::endl;
                return result;
            }
            if (surfaceCapabilities.currentExtent.width == 0 ||
                surfaceCapabilities.currentExtent.height == 0)
                return VK_SUCCESS;
            swapchainCreateInfo.imageExtent = surfaceCapabilities.currentExtent;
            swapchainCreateInfo.oldSwapchain = swapchain;

            VkResult result = vkQueueWaitIdle(queueGraphics);
            if (!result &&
                queueGraphics != queuePresentation)
                result = vkQueueWaitIdle(queuePresentation);
            if (result) {
                std::cerr << std::format("Failed to wait for the queue to be idle.\nError code: {}", int32_t(result)) << std::endl;
                return result;
            }

            for (auto& i : callbacksDestroySwapchain) i();

            for (auto& i : swapchainImageViews)
                if (i)
                    vkDestroyImageView(device, i, nullptr);
            swapchainImageViews.resize(0);

            if (result = createSwapchainInternal())
                return result;

            for (auto& i : callbacksCreateSwapchain) i();

            return VK_SUCCESS;
        }

    private:
        std::vector<void(*)()> callbacksCreateSwapchain;
        std::vector<void(*)()> callbacksDestroySwapchain;

    public:
        void pushCallbackCreateSwapchain(void(*function)()) {
            callbacksCreateSwapchain.push_back(function);
        }
        void pushCallbackDestroySwapchain(void(*function)()) {
            callbacksDestroySwapchain.push_back(function);
        }

        VkResult waitIdle() const {
            VkResult result = vkDeviceWaitIdle(device);
            if (result)
                std::cout << std::format("Failed to wait for the device to be idle.\nError code: {}", int32_t(result)) << std::endl;
            return result;
        }

    public:
        void terminate() {
            this->~GraphicsBase();
            instance = VK_NULL_HANDLE;
            physicalDevice = VK_NULL_HANDLE;
            device = VK_NULL_HANDLE;
            surface = VK_NULL_HANDLE;
            swapchain = VK_NULL_HANDLE;
            swapchainImages.resize(0);
            swapchainImageViews.resize(0);
            swapchainCreateInfo = {};
            debugUtilsMessenger = VK_NULL_HANDLE;
        }
    };


    inline GraphicsBase GraphicsBase::singleton;


}