/**
 * @purpose vulkan graphics setup and abstraction
*/

#include <SDL.h>
#include <SDL_vulkan.h>
#include <vulkan/vulkan.h>

#include "gf3d_vector.h"
#include "gf3d_types.h"
#include "simple_logger.h"

typedef struct
{
    SDL_Window * main_window;
    VkApplicationInfo vk_app_info;
    VkInstance vk_instance;
    VkInstanceCreateInfo vk_instance_info;
    unsigned int enabled_extension_count;
    const char **enabled_extension_names;
    unsigned int enabled_layer_count;
    Uint32 device_count;
    VkPhysicalDevice *devices;
    VkPhysicalDevice gpu;
    Uint32 queue_property_count;
    VkQueueFamilyProperties *queue_properties;
}vGraphics;

static vGraphics gf3d_vgraphics = {0};


void gf3d_vgraphics_close();


void gf3d_vgraphics_init(
    char *windowName,
    int renderWidth,
    int renderHeight,
    Vector4D bgcolor,
    Bool fullscreen
)
{
    Uint32 flags = SDL_WINDOW_VULKAN;
    Uint32 i;
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        slog("Unable to initilaize SDL system: %s",SDL_GetError());
        return;
    }
    atexit(SDL_Quit);
    if (fullscreen)
    {
        if (renderWidth == 0)
        {
            flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
        }
        else
        {
            flags |= SDL_WINDOW_FULLSCREEN;
        }
    }
    gf3d_vgraphics.main_window = SDL_CreateWindow(windowName,
                             SDL_WINDOWPOS_UNDEFINED,
                             SDL_WINDOWPOS_UNDEFINED,
                             renderWidth, renderHeight,
                             flags);

    if (!gf3d_vgraphics.main_window)
    {
        slog("failed to create main window: %s",SDL_GetError());
        gf3d_vgraphics_close();
        return;
    }
    
    SDL_Vulkan_GetInstanceExtensions(gf3d_vgraphics.main_window, &(gf3d_vgraphics.enabled_extension_count), NULL);
    if (gf3d_vgraphics.enabled_extension_count > 0)
    {
        gf3d_vgraphics.enabled_extension_names = malloc(sizeof(const char *) * gf3d_vgraphics.enabled_extension_count);
        SDL_Vulkan_GetInstanceExtensions(gf3d_vgraphics.main_window, &(gf3d_vgraphics.enabled_extension_count), gf3d_vgraphics.enabled_extension_names);
        for (i = 0; i < gf3d_vgraphics.enabled_extension_count;i++)
        {
            slog("extensions loaded: %s",gf3d_vgraphics.enabled_extension_names[i]);
        }
    }
    // setup app info
    gf3d_vgraphics.vk_app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    gf3d_vgraphics.vk_app_info.pNext = NULL;
    gf3d_vgraphics.vk_app_info.pApplicationName = windowName;
    gf3d_vgraphics.vk_app_info.applicationVersion = 0;
    gf3d_vgraphics.vk_app_info.pEngineName = windowName;
    gf3d_vgraphics.vk_app_info.engineVersion = 0;
    gf3d_vgraphics.vk_app_info.apiVersion = VK_API_VERSION_1_0;
    //setup instance info
    gf3d_vgraphics.vk_instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    gf3d_vgraphics.vk_instance_info.pNext = NULL;
    gf3d_vgraphics.vk_instance_info.pApplicationInfo = &gf3d_vgraphics.vk_app_info;
    gf3d_vgraphics.vk_instance_info.enabledLayerCount = 0;
    gf3d_vgraphics.vk_instance_info.ppEnabledLayerNames = NULL;
    gf3d_vgraphics.vk_instance_info.enabledExtensionCount = gf3d_vgraphics.enabled_extension_count;
    gf3d_vgraphics.vk_instance_info.ppEnabledExtensionNames = gf3d_vgraphics.enabled_extension_names;

    // create instance
    vkCreateInstance(&gf3d_vgraphics.vk_instance_info, NULL, &gf3d_vgraphics.vk_instance);
    
    if (!gf3d_vgraphics.vk_instance)
    {
        slog("failed to create a vulkan instance");
        gf3d_vgraphics_close();
        return;
    }
    
    //get a gpu to do work with
    vkEnumeratePhysicalDevices(gf3d_vgraphics.vk_instance, &gf3d_vgraphics.device_count, NULL);
    slog("vulkan discovered %i device(s) with this instance",gf3d_vgraphics.device_count);
    if (!gf3d_vgraphics.device_count)
    {
        slog("failed to create a vulkan instance with a usable device");
        gf3d_vgraphics_close();
        return;
    }
    gf3d_vgraphics.devices = (VkPhysicalDevice *)malloc(sizeof(VkPhysicalDevice)*gf3d_vgraphics.device_count);
    vkEnumeratePhysicalDevices(gf3d_vgraphics.vk_instance, &gf3d_vgraphics.device_count, gf3d_vgraphics.devices);
    
    gf3d_vgraphics.gpu = gf3d_vgraphics.devices[0];
    
    // setup queues
    
    vkGetPhysicalDeviceQueueFamilyProperties(
        gf3d_vgraphics.gpu,
        &gf3d_vgraphics.queue_property_count,
        NULL);
    
    if (!gf3d_vgraphics.queue_property_count)
    {
        slog("failed to get any queue properties");
        gf3d_vgraphics_close();
        return;
    }
    gf3d_vgraphics.queue_properties = (VkQueueFamilyProperties*)malloc(sizeof(VkQueueFamilyProperties) * gf3d_vgraphics.queue_property_count);
    vkGetPhysicalDeviceQueueFamilyProperties(
        gf3d_vgraphics.gpu,
        &gf3d_vgraphics.queue_property_count,
        gf3d_vgraphics.queue_properties);
    slog("discoverd %i queue family properties",gf3d_vgraphics.queue_property_count);
    for (i = 0; i < gf3d_vgraphics.queue_property_count; i++)
    {
        slog("Queue family %i:",i);
        slog("queue flag bits %i",gf3d_vgraphics.queue_properties[i].queueFlags);
        slog("queue count %i",gf3d_vgraphics.queue_properties[i].queueCount);
        slog("queue timestamp valid bits %i",gf3d_vgraphics.queue_properties[i].timestampValidBits);
        slog("queue min image transfer granularity %iw %ih %id",
             gf3d_vgraphics.queue_properties[i].minImageTransferGranularity.width,
             gf3d_vgraphics.queue_properties[i].minImageTransferGranularity.height,
             gf3d_vgraphics.queue_properties[i].minImageTransferGranularity.depth);
    }
}

void gf3d_vgraphics_close()
{
    if (gf3d_vgraphics.main_window)
    {
        SDL_DestroyWindow(gf3d_vgraphics.main_window);
    }
}

void gf3d_vgraphics_clear()
{
    
}

void gf3d_vgraphics_render()
{
    
}
/*eol@eof*/

