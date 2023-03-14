#ifndef HELLOTRIANGLE_EXTERNS_H
#define HELLOTRIANGLE_EXTERNS_H

#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstring>
#include <set>
#include <string>
#include <stdexcept>
#include <algorithm>

#include "QueueFamilyIndices.h"
#include "SwapChainSupportDetails.h"

extern const std::vector<const char*> g_validationLayers;

extern const std::vector<const char*> g_deviceExtensions;

extern const bool g_enableValidationLayers;

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
                                      const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator,
                                      VkDebugUtilsMessengerEXT* pDebugMessenger);

void DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                   VkDebugUtilsMessengerEXT debugMessenger,
                                   const VkAllocationCallbacks* pAllocator);


#endif