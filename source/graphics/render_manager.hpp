#pragma once

#include <vector>
#include <vulkan/vulkan_core.h>


class SDL_Window;
class SDL_Surface;


class Render_manager
{
public:

	bool startup();
	void shutdown();
	void update();

private:

	SDL_Window*              window = nullptr;
	VkSurfaceKHR             surface;
	VkInstance               vulkan_instance;
	VkDebugUtilsMessengerEXT debug_messenger;

	bool create_vulkan_instance();
	bool check_validation_layer_support();

	std::vector<const char*> get_required_extensions();

	void setup_debug_messenger();

	static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT      message_severity,
	                                                     VkDebugUtilsMessageTypeFlagsEXT             message_type,
	                                                     const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
	                                                     void*                                       user_data);
	static VkResult
	create_debug_utils_messenger_ext(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* create_info, const VkAllocationCallbacks* allocator, VkDebugUtilsMessengerEXT* debug_messenger);
	static void populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT& create_info);
	static void destroy_debug_utils_messenger_ext(VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger, const VkAllocationCallbacks* allocator);
};