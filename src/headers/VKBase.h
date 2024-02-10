#pragma once

#include "./VKUtil.h"
#include <vulkan/vulkan_core.h>

namespace Vulkan {
    class GraphicsBase {

        static GraphicsBase singleton;
        GraphicsBase() = default;
        GraphicsBase(GraphicsBase&&) = delete;
        ~GraphicsBase() {

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

        }

        VkResult setSurfaceFormat(VkSurfaceFormatKHR surfaceFormat) {

        }

        VkResult createSwapchain(bool limitFrameRate = true, const void* pNext = nullptr, VkSwapchainCreateFlagsKHR flags = 0) {

        }
        
        VkResult recreateSwapchain() {
        
        }
    };

    inline GraphicsBase GraphicsBase::singleton;


}