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