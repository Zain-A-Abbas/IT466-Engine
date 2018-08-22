#ifndef __GF3D_SWAPCHAIN_H__
#define __GF3D_SWAPCHAIN_H__

#include <vulkan/vulkan.h>

#include "gf3d_types.h"

/**
 * @brief validate and setup swap chain
 * @param device the physical device to init the swap chain for
 * @param logicalDevice the logical device to make the swap chain for
 * @param surface the surface that the swap chain should support
 * @param width the desired width of the swap chain buffers
 * @param height the desired height of the swap chain buffers
 */
void gf3d_swapchain_init(VkPhysicalDevice device,VkDevice logicalDevice, VkSurfaceKHR surface,Uint32 width,Uint32 height);

/**
 * @brief check if the initialized swap chain is sufficient for rendering
 * @returns false if not, true if it will work for rendering
 */
Bool gf3d_swapchain_validation_check();

/**
 * @brief called at exit to clean up the swap chains
 */
void gf3d_swapchain_close();

#endif
