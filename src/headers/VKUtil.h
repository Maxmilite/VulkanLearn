#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <stack>
#include <map>
#include <unordered_map>
#include <span>
#include <memory>
#include <functional>
#include <concepts>
#include <format>
#include <chrono>
#include <numeric>
#include <numbers>
#include <stdexcept>

// GLM
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

// stb_image.h
#include <stb_image.h>

// Vulkan
#include <vulkan/vulkan.hpp>
#pragma comment(lib, "vulkan-1.lib")

// I18N
#include "./I18N.h"

// Exception Handling
#ifdef VK_RESULT_THROW
class result_t {
    VkResult result;
public:
    static void(*callback_throw)(VkResult);
    result_t(VkResult result) :result(result) {}
    result_t(result_t&& other) noexcept :result(other.result) { other.result = VK_SUCCESS; }
    ~result_t() noexcept(false) {
        if (uint32_t(result) < VK_RESULT_MAX_ENUM)
            return;
        if (callback_throw)
            callback_throw(result);
        throw result;
    }
    operator VkResult() {
        VkResult result = this->result;
        this->result = VK_SUCCESS;
        return result;
    }
};
inline void(*result_t::callback_throw)(VkResult);

#elif defined VK_RESULT_NODISCARD
struct [[nodiscard]] result_t {
    VkResult result;
    result_t(VkResult result) :result(result) {}
    operator VkResult() const { return result; }
};
#pragma warning(disable:4834)
#pragma warning(disable:6031)

#else
using result_t = VkResult;
#endif

// Utility
#ifndef NDEBUG
#define ENABLE_DEBUG_MESSENGER true
#else
#define ENABLE_DEBUG_MESSENGER false
#endif

#define destroyHandleBy(func) if (handle) { func(GraphicsBase::getBase().getDevice(), handle, nullptr); handle = VK_NULL_HANDLE; }
#define moveHandle do { handle = other.handle; other.handle = VK_NULL_HANDLE; } while (0)
#define defineMoveAssignmentOperator(type) type& operator=(type&& other) { this->~type(); moveHandle; return *this; }
#define defineHandleTypeOperator operator decltype(handle)() const { return handle; }
#define defineAddressFunction const decltype(handle)* address() const { return &handle; }

#define executeOnce(...) { static bool executed = false; if (executed) return __VA_ARGS__; executed = true; }
