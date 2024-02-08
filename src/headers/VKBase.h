#pragma once

#include "./VKUtil.h"

namespace Vulkan {
    class GraphicsBase {

        static GraphicsBase singleton;
        GraphicsBase() = default;
        GraphicsBase(GraphicsBase&&) = delete;
        ~GraphicsBase() {

        }
    
    public:
        static GraphicsBase& base() {
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
        
        }
        
        VkResult checkInstanceLayers(std::span<const char*> layersToCheck) {
        
        }

        void setInstanceLayers(const std::vector<const char*>& layerNames) {
            instanceLayers = layerNames;
        }

        VkResult checkInstanceExtensions(std::span<const char*> extensionsToCheck, const char* layerName = nullptr) const {

        }

        void setInstanceExtensions(const std::vector<const char*>& extensionNames) {
            instanceExtensions = extensionNames;
        }
    
    // Debug Messenger
    private:
        VkDebugUtilsMessengerEXT debugUtilsMessenger;
        VkResult CreateDebugMessenger() {
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
        
        }

        VkResult determinePhysicalDevice(uint32_t deviceIndex = 0, bool enableGraphicsQueue = true, bool enableComputeQueue = true) {
        
        }

        VkResult createDevice(const void* pNext = nullptr, VkDeviceCreateFlags flags = 0) {
        
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