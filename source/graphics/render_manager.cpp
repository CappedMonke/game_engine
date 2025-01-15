#include "render_manager.hpp"

#include <SDL3/SDL_log.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_vulkan.h>

#include "config/application.hpp"


#ifdef NDEBUG
const bool enable_validation_layers = false;
#else
const bool enable_validation_layers = true;
#endif


const std::vector<const char*> validation_layers = {"VK_LAYER_KHRONOS_validation"};


bool Render_manager::startup()
{
	window = SDL_CreateWindow(GAME_NAME, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_VULKAN & SDL_WINDOW_RESIZABLE);
	if (!window)
	{
		return false;
	}

	if (!create_vulkan_instance())
	{
		return false;
	}

	setup_debug_messenger();

	return true;
}

void Render_manager::shutdown()
{
	vkDestroyInstance(vulkan_instance, nullptr);
	SDL_DestroyWindow(window);
}

void Render_manager::update()
{
}

bool Render_manager::create_vulkan_instance()
{
	if (enable_validation_layers && !check_validation_layer_support())
	{
		return false;
	}

	VkApplicationInfo app_info  = {};
	app_info.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pApplicationName   = GAME_NAME;
	app_info.applicationVersion = VK_MAKE_API_VERSION(0, 0, 0, 0);
	app_info.pEngineName        = "No Engine";
	app_info.engineVersion      = VK_MAKE_API_VERSION(0, 0, 0, 0);
	app_info.apiVersion         = VK_API_VERSION_1_0;

	std::vector<const char*> extensions = get_required_extensions();

	VkInstanceCreateInfo create_info    = {};
	create_info.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	create_info.pApplicationInfo        = &app_info;
	create_info.enabledExtensionCount   = static_cast<uint32_t>(extensions.size());
	create_info.ppEnabledExtensionNames = extensions.data();
	if (enable_validation_layers)
	{
		create_info.enabledLayerCount   = static_cast<uint32_t>(validation_layers.size());
		create_info.ppEnabledLayerNames = validation_layers.data();
	}

	VkResult result = vkCreateInstance(&create_info, nullptr, &vulkan_instance);
	if (result != VK_SUCCESS)
	{
		SDL_SetError("Failed to create Vulkan instance: %d", result);
		return false;
	}

	return true;
}

bool Render_manager::check_validation_layer_support()
{
	uint32_t layer_count;
	vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

	std::vector<VkLayerProperties> available_layers(layer_count);
	vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

	for (const char* layer_name : validation_layers)
	{
		bool layer_found = false;

		for (const VkLayerProperties& layer_properties : available_layers)
		{
			if (strcmp(layer_name, layer_properties.layerName) == 0)
			{
				layer_found = true;
				break;
			}
		}

		if (!layer_found)
		{
			SDL_SetError("Validation layer not found: %s", layer_name);
			return false;
		}
	}

	return true;
}

std::vector<const char*> Render_manager::get_required_extensions()
{
	uint32_t           sdl_extension_count = 0;
	const char* const* sdl_extensions      = SDL_Vulkan_GetInstanceExtensions(&sdl_extension_count);

	std::vector<const char*> extensions(sdl_extensions, sdl_extensions + sdl_extension_count);

	if (enable_validation_layers)
	{
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}

void Render_manager::setup_debug_messenger()
{
	if (!enable_validation_layers)
	{
		return;
	}

	VkDebugUtilsMessengerCreateInfoEXT create_info = {};
	create_info.sType                              = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	create_info.messageSeverity                    = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
	create_info.messageType                        = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
	create_info.pfnUserCallback                    = debug_callback;
}

VKAPI_ATTR VkBool32 VKAPI_CALL Render_manager::debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT      message_severity,
                                                              VkDebugUtilsMessageTypeFlagsEXT             message_type,
                                                              const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
                                                              void*                                       user_data)
{
	SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Validation layer: %s", callback_data->pMessage);
	return VK_FALSE;
}