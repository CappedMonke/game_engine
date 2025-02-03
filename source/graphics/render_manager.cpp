#include "render_manager.hpp"

#include <SDL3/SDL_log.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_vulkan.h>
#include <algorithm>
#include <fstream>
#include <limits>
#include <set>

#include "config/application.hpp"
#include <string>


#ifdef NDEBUG
const bool enable_validation_layers = false;
#else
const bool enable_validation_layers = true;
#endif


const std::vector<const char*> validation_layers    = {"VK_LAYER_KHRONOS_validation"};
const std::vector<const char*> device_extensions    = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
const int                      MAX_FRAMES_IN_FLIGHT = 2;

bool Render_manager::startup()
{
	window = SDL_CreateWindow(GAME_NAME, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_VULKAN);
	if (!window)
	{
		return false;
	}

	if (!create_vulkan_instance())
	{
		return false;
	}
	setup_debug_messenger();
	create_surface();
	pick_physical_device();
	create_logical_device();
	create_swapchain();
	create_image_views();
	create_render_pass();
	create_graphics_pipeline();
	create_frame_buffers();
	create_command_pool();
	create_command_buffers();
	create_sync_objects();

	return true;
}

void Render_manager::shutdown()
{
	cleanup_swapchain();

	vkDestroyPipeline(device, graphics_pipeline, nullptr);
	vkDestroyPipelineLayout(device, pipeline_layout, nullptr);

	vkDestroyRenderPass(device, render_pass, nullptr);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroySemaphore(device, image_available_semaphores[i], nullptr);
		vkDestroySemaphore(device, render_finished_semaphores[i], nullptr);
		vkDestroyFence(device, in_flight_fences[i], nullptr);
	}

	vkDestroyCommandPool(device, command_pool, nullptr);

	vkDestroyDevice(device, nullptr);

	if (enable_validation_layers)
	{
		destroy_debug_utils_messenger_ext(vulkan_instance, debug_messenger, nullptr);
	}

	vkDestroyInstance(vulkan_instance, nullptr);
	SDL_Vulkan_DestroySurface(vulkan_instance, surface, nullptr);

	SDL_DestroyWindow(window);
}

void Render_manager::update()
{
	draw_frame();
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

	VkDebugUtilsMessengerCreateInfoEXT debug_create_info = {};
	if (enable_validation_layers)
	{
		create_info.enabledLayerCount   = static_cast<uint32_t>(validation_layers.size());
		create_info.ppEnabledLayerNames = validation_layers.data();
		populate_debug_messenger_create_info(debug_create_info);
		create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debug_create_info;
	}

	VkResult result = vkCreateInstance(&create_info, nullptr, &vulkan_instance);
	if (result != VK_SUCCESS)
	{
		SDL_SetError("Failed to create Vulkan instance: %d", result);
		return false;
	}

	return true;
}

void Render_manager::create_surface()
{
	if (!SDL_Vulkan_CreateSurface(window, vulkan_instance, nullptr, &surface))
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create window surface: %s", SDL_GetError());
	}
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

bool Render_manager::check_device_extension_support(VkPhysicalDevice device)
{
	uint32_t extension_count;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);

	std::vector<VkExtensionProperties> available_extensions(extension_count);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, available_extensions.data());

	std::set<std::string> required_extensions(device_extensions.begin(), device_extensions.end());

	for (const auto& extension : available_extensions)
	{
		required_extensions.erase(extension.extensionName);
	}

	return required_extensions.empty();
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

	VkDebugUtilsMessengerCreateInfoEXT create_info;
	populate_debug_messenger_create_info(create_info);

	if (create_debug_utils_messenger_ext(vulkan_instance, &create_info, nullptr, &debug_messenger) != VK_SUCCESS)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create debug messenger.");
	}
}

VKAPI_ATTR VkBool32 VKAPI_CALL Render_manager::debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT      message_severity,
                                                              VkDebugUtilsMessageTypeFlagsEXT             message_type,
                                                              const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
                                                              void*                                       user_data)
{
	SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Validation layer: %s", callback_data->pMessage);
	return VK_FALSE;
}

VkResult Render_manager::create_debug_utils_messenger_ext(VkInstance                                vulkan_instance,
                                                          const VkDebugUtilsMessengerCreateInfoEXT* create_info,
                                                          const VkAllocationCallbacks*              allocator,
                                                          VkDebugUtilsMessengerEXT*                 debug_messenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vulkan_instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		return func(vulkan_instance, create_info, allocator, debug_messenger);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void Render_manager::populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT& create_info)
{
	create_info                 = {};
	create_info.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	create_info.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	create_info.pfnUserCallback = debug_callback;
}

void Render_manager::destroy_debug_utils_messenger_ext(VkInstance vulkan_instance, VkDebugUtilsMessengerEXT debug_messenger, const VkAllocationCallbacks* allocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vulkan_instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		func(vulkan_instance, debug_messenger, allocator);
	}
}

void Render_manager::pick_physical_device()
{
	uint32_t device_count = 0;
	vkEnumeratePhysicalDevices(vulkan_instance, &device_count, nullptr);

	if (device_count == 0)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to find GPUs with Vulkan support.");
	}

	std::vector<VkPhysicalDevice> devices(device_count);
	vkEnumeratePhysicalDevices(vulkan_instance, &device_count, devices.data());

	for (const VkPhysicalDevice& device : devices)
	{
		if (is_device_suitable(device))
		{
			physical_device = device;
			break;
		}
	}

	if (physical_device == VK_NULL_HANDLE)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to find a suitable GPU.");
	}
}

bool Render_manager::is_device_suitable(VkPhysicalDevice device)
{
	Queue_family_indices indices = find_queue_families(device);

	bool extensions_supported = check_device_extension_support(device);

	bool swap_chain_adequate = false;
	if (extensions_supported)
	{
		Swap_chain_support_details swap_chain_support = query_swap_chain_support(device);
		swap_chain_adequate                           = !swap_chain_support.formats.empty() && !swap_chain_support.present_modes.empty();
	}

	return indices.is_complete() && extensions_supported && swap_chain_adequate;
}

Queue_family_indices Render_manager::find_queue_families(VkPhysicalDevice device)
{
	Queue_family_indices indices;

	uint32_t queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);

	std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());

	int i = 0;
	for (const VkQueueFamilyProperties& queue_family : queue_families)
	{
		if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphics_family = i;
		}

		VkBool32 present_support = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);
		if (present_support)
		{
			indices.present_family = i;
		}

		if (indices.is_complete())
		{
			break;
		}

		i++;
	}

	return indices;
}

void Render_manager::create_logical_device()
{
	Queue_family_indices indices = find_queue_families(physical_device);

	std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
	std::set<uint32_t>                   unique_queue_families = {indices.graphics_family.value(), indices.present_family.value()};

	float queue_priority = 1.0f;
	for (uint32_t queue_family : unique_queue_families)
	{
		VkDeviceQueueCreateInfo queue_create_info = {};
		queue_create_info.sType                   = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_create_info.queueFamilyIndex        = queue_family;
		queue_create_info.queueCount              = 1;
		queue_create_info.pQueuePriorities        = &queue_priority;
		queue_create_infos.push_back(queue_create_info);
	}

	VkPhysicalDeviceFeatures device_features = {};

	VkDeviceCreateInfo create_info      = {};
	create_info.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	create_info.pQueueCreateInfos       = queue_create_infos.data();
	create_info.queueCreateInfoCount    = static_cast<uint32_t>(queue_create_infos.size());
	create_info.pEnabledFeatures        = &device_features;
	create_info.enabledExtensionCount   = static_cast<uint32_t>(device_extensions.size());
	create_info.ppEnabledExtensionNames = device_extensions.data();
	if (enable_validation_layers)
	{
		create_info.enabledLayerCount   = static_cast<uint32_t>(validation_layers.size());
		create_info.ppEnabledLayerNames = validation_layers.data();
	}

	if (vkCreateDevice(physical_device, &create_info, nullptr, &device) != VK_SUCCESS)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create logical device");
	}

	vkGetDeviceQueue(device, indices.graphics_family.value(), 0, &graphics_queue);
	vkGetDeviceQueue(device, indices.present_family.value(), 0, &present_queue);
}

Swap_chain_support_details Render_manager::query_swap_chain_support(VkPhysicalDevice device)
{
	Swap_chain_support_details details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

	uint32_t format_count;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, nullptr);

	if (format_count != 0)
	{
		details.formats.resize(format_count);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, details.formats.data());
	}

	uint32_t present_mode_count;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, nullptr);

	if (present_mode_count != 0)
	{
		details.present_modes.resize(present_mode_count);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, details.present_modes.data());
	}

	return details;
}

VkSurfaceFormatKHR Render_manager::choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& available_formats)
{
	for (const auto& available_format : available_formats)
	{
		if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB && available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return available_format;
		}
	}

	return available_formats[0];
}

VkPresentModeKHR Render_manager::choose_swap_present_mode(const std::vector<VkPresentModeKHR>& available_present_modes)
{
	for (const auto& available_present_mode : available_present_modes)
	{
		if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return available_present_mode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Render_manager::choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return capabilities.currentExtent;
	}
	else
	{
		int width;
		int height;
		SDL_GetWindowSizeInPixels(window, &width, &height);

		VkExtent2D actual_extent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

		actual_extent.width  = std::clamp(actual_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actual_extent.height = std::clamp(actual_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actual_extent;
	}
}

void Render_manager::create_swapchain()
{
	Swap_chain_support_details swap_chain_support = query_swap_chain_support(physical_device);

	VkSurfaceFormatKHR surface_format = choose_swap_surface_format(swap_chain_support.formats);
	VkPresentModeKHR   present_mode   = choose_swap_present_mode(swap_chain_support.present_modes);
	VkExtent2D         extent         = choose_swap_extent(swap_chain_support.capabilities);

	uint32_t image_count = swap_chain_support.capabilities.minImageCount + 1;
	if (swap_chain_support.capabilities.maxImageCount > 0 && image_count > swap_chain_support.capabilities.maxImageCount)
	{
		image_count = swap_chain_support.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR create_info = {};
	create_info.sType                    = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	create_info.surface                  = surface;
	create_info.minImageCount            = image_count;
	create_info.imageFormat              = surface_format.format;
	create_info.imageColorSpace          = surface_format.colorSpace;
	create_info.imageExtent              = extent;
	create_info.imageArrayLayers         = 1;
	create_info.imageUsage               = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	Queue_family_indices indices                = find_queue_families(physical_device);
	uint32_t             queue_family_indices[] = {indices.graphics_family.value(), indices.present_family.value()};
	if (indices.graphics_family != indices.present_family)
	{
		create_info.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
		create_info.queueFamilyIndexCount = 2;
		create_info.pQueueFamilyIndices   = queue_family_indices;
	}
	else
	{
		create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	create_info.preTransform   = swap_chain_support.capabilities.currentTransform;
	create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	create_info.presentMode    = present_mode;
	create_info.clipped        = VK_TRUE;
	create_info.oldSwapchain   = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(device, &create_info, nullptr, &swap_chain))
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create swap chain.");
	}

	vkGetSwapchainImagesKHR(device, swap_chain, &image_count, nullptr);
	swap_chain_images.resize(image_count);
	vkGetSwapchainImagesKHR(device, swap_chain, &image_count, swap_chain_images.data());

	swap_chain_image_format = surface_format.format;
	swap_chain_extent       = extent;
}

void Render_manager::recreate_swapchain()
{
	vkDeviceWaitIdle(device);

	cleanup_swapchain();

	create_swapchain();
	create_image_views();
	create_frame_buffers();
}

void Render_manager::cleanup_swapchain()
{
	for (auto framebuffer : swap_chain_frame_buffers)
	{
		vkDestroyFramebuffer(device, framebuffer, nullptr);
	}

	for (auto image_view : swap_chain_image_views)
	{
		vkDestroyImageView(device, image_view, nullptr);
	}

	vkDestroySwapchainKHR(device, swap_chain, nullptr);
}

void Render_manager::create_image_views()
{
	swap_chain_image_views.resize(swap_chain_images.size());

	for (size_t i = 0; i < swap_chain_images.size(); i++)
	{
		VkImageViewCreateInfo create_info           = {};
		create_info.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		create_info.image                           = swap_chain_images[i];
		create_info.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
		create_info.format                          = swap_chain_image_format;
		create_info.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
		create_info.subresourceRange.baseMipLevel   = 0;
		create_info.subresourceRange.levelCount     = 1;
		create_info.subresourceRange.baseArrayLayer = 0;
		create_info.subresourceRange.layerCount     = 1;

		if (vkCreateImageView(device, &create_info, nullptr, &swap_chain_image_views[i]) != VK_SUCCESS)
		{
			SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create image view.");
		}
	}
}

void Render_manager::create_graphics_pipeline()
{
	auto vert_shader_code = read_file("shaders/vert.spv");
	auto frag_shader_code = read_file("shaders/frag.spv");

	VkShaderModule vert_shader_module = create_shader_module(vert_shader_code);
	VkShaderModule frag_shader_module = create_shader_module(frag_shader_code);

	VkPipelineShaderStageCreateInfo vert_shader_stage_info = {};
	vert_shader_stage_info.sType                           = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vert_shader_stage_info.stage                           = VK_SHADER_STAGE_VERTEX_BIT;
	vert_shader_stage_info.module                          = vert_shader_module;
	vert_shader_stage_info.pName                           = "main";

	VkPipelineShaderStageCreateInfo frag_shader_stage_info = {};
	frag_shader_stage_info.sType                           = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	frag_shader_stage_info.stage                           = VK_SHADER_STAGE_FRAGMENT_BIT;
	frag_shader_stage_info.module                          = frag_shader_module;
	frag_shader_stage_info.pName                           = "main";

	VkPipelineShaderStageCreateInfo shader_stages[] = {vert_shader_stage_info, frag_shader_stage_info};

	std::vector<VkDynamicState> dynamic_states = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

	VkPipelineDynamicStateCreateInfo dynamic_state = {};
	dynamic_state.sType                            = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic_state.dynamicStateCount                = static_cast<uint32_t>(dynamic_states.size());
	dynamic_state.pDynamicStates                   = dynamic_states.data();

	VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
	vertex_input_info.sType                                = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_input_info.vertexBindingDescriptionCount        = 0;
	vertex_input_info.vertexAttributeDescriptionCount      = 0;

	VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
	input_assembly.sType                                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly.topology                               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	input_assembly.primitiveRestartEnable                 = VK_FALSE;

	VkViewport viewport = {};
	viewport.x          = 0.0f;
	viewport.y          = 0.0f;
	viewport.height     = (float)swap_chain_extent.width;
	viewport.width      = (float)swap_chain_extent.height;
	viewport.minDepth   = 0.0f;
	viewport.maxDepth   = 1.0f;

	VkRect2D scissors = {};
	scissors.offset   = {0, 0};
	scissors.extent   = swap_chain_extent;

	VkPipelineViewportStateCreateInfo viewport_state = {};
	viewport_state.sType                             = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state.viewportCount                     = 1;
	viewport_state.pViewports                        = &viewport;
	viewport_state.scissorCount                      = 1;
	viewport_state.pScissors                         = &scissors;

	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType                                  = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable                       = VK_FALSE;
	rasterizer.rasterizerDiscardEnable                = VK_FALSE;
	rasterizer.polygonMode                            = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth                              = 1.0f;
	rasterizer.cullMode                               = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace                              = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable                        = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType                                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable                  = VK_FALSE;
	multisampling.rasterizationSamples                 = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineColorBlendAttachmentState color_blend_attachment = {};
	color_blend_attachment.colorWriteMask                      = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	color_blend_attachment.blendEnable                         = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo color_blending = {};
	color_blending.sType                               = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blending.logicOpEnable                       = VK_FALSE;
	color_blending.attachmentCount                     = 1;
	color_blending.pAttachments                        = &color_blend_attachment;

	VkPipelineLayoutCreateInfo pipeline_create_info = {};
	pipeline_create_info.sType                      = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

	if (vkCreatePipelineLayout(device, &pipeline_create_info, nullptr, &pipeline_layout) != VK_SUCCESS)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create pipeline layout.");
	}

	VkGraphicsPipelineCreateInfo pipeline_info = {};
	pipeline_info.sType                        = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_info.stageCount                   = 2;
	pipeline_info.pStages                      = shader_stages;
	pipeline_info.pVertexInputState            = &vertex_input_info;
	pipeline_info.pInputAssemblyState          = &input_assembly;
	pipeline_info.pViewportState               = &viewport_state;
	pipeline_info.pRasterizationState          = &rasterizer;
	pipeline_info.pMultisampleState            = &multisampling;
	pipeline_info.pColorBlendState             = &color_blending;
	pipeline_info.pDynamicState                = &dynamic_state;
	pipeline_info.layout                       = pipeline_layout;
	pipeline_info.renderPass                   = render_pass;
	pipeline_info.subpass                      = 0;

	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &graphics_pipeline))
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create pipeline.");
	}

	vkDestroyShaderModule(device, vert_shader_module, nullptr);
	vkDestroyShaderModule(device, frag_shader_module, nullptr);
}

std::vector<char> Render_manager::read_file(const std::string& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to open file %s.", filename.c_str());
	}

	size_t            file_size = (size_t)file.tellg();
	std::vector<char> buffer(file_size);

	file.seekg(0);
	file.read(buffer.data(), file_size);
	file.close();

	return buffer;
}

VkShaderModule Render_manager::create_shader_module(const std::vector<char>& code)
{
	VkShaderModuleCreateInfo create_info = {};
	create_info.sType                    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	create_info.codeSize                 = code.size();
	create_info.pCode                    = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shader_module;
	if (vkCreateShaderModule(device, &create_info, nullptr, &shader_module) != VK_SUCCESS)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create shader module.");
	}

	return shader_module;
}

void Render_manager::create_render_pass()
{
	VkAttachmentDescription color_attachment = {};
	color_attachment.format                  = swap_chain_image_format;
	color_attachment.samples                 = VK_SAMPLE_COUNT_1_BIT;
	color_attachment.loadOp                  = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment.storeOp                 = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment.stencilLoadOp           = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp          = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment.initialLayout           = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment.finalLayout             = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference color_attachment_ref = {};
	color_attachment_ref.attachment            = 0;
	color_attachment_ref.layout                = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass          = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass          = 0;
	dependency.srcStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask       = 0;
	dependency.dstStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask       = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments    = &color_attachment_ref;

	VkRenderPassCreateInfo render_pass_info = {};
	render_pass_info.sType                  = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_info.attachmentCount        = 1;
	render_pass_info.pAttachments           = &color_attachment;
	render_pass_info.subpassCount           = 1;
	render_pass_info.pSubpasses             = &subpass;
	render_pass_info.dependencyCount        = 1;
	render_pass_info.pDependencies          = &dependency;

	if (vkCreateRenderPass(device, &render_pass_info, nullptr, &render_pass))
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create render pass.");
	}
}

void Render_manager::create_frame_buffers()
{
	swap_chain_frame_buffers.resize(swap_chain_image_views.size());

	for (uint32_t i = 0; i < swap_chain_image_views.size(); i++)
	{
		VkImageView attachments[] = {swap_chain_image_views[i]};

		VkFramebufferCreateInfo frame_buffer_info = {};
		frame_buffer_info.sType                   = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frame_buffer_info.renderPass              = render_pass;
		frame_buffer_info.attachmentCount         = 1;
		frame_buffer_info.pAttachments            = attachments;
		frame_buffer_info.width                   = swap_chain_extent.width;
		frame_buffer_info.height                  = swap_chain_extent.height;
		frame_buffer_info.layers                  = 1;

		if (vkCreateFramebuffer(device, &frame_buffer_info, nullptr, &swap_chain_frame_buffers[i]) != VK_SUCCESS)
		{
			SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create framebuffer");
		}
	}
}

void Render_manager::create_command_pool()
{
	Queue_family_indices queue_family_indices = find_queue_families(physical_device);

	VkCommandPoolCreateInfo pool_info = {};
	pool_info.sType                   = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool_info.flags                   = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	pool_info.queueFamilyIndex        = queue_family_indices.graphics_family.value();

	if (vkCreateCommandPool(device, &pool_info, nullptr, &command_pool) != VK_SUCCESS)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create command pool");
	}
}

void Render_manager::create_command_buffers()
{
	command_buffers.resize(MAX_FRAMES_IN_FLIGHT);

	VkCommandBufferAllocateInfo alloc_info = {};
	alloc_info.sType                       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.commandPool                 = command_pool;
	alloc_info.level                       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc_info.commandBufferCount          = (uint32_t)command_buffers.size();

	if (vkAllocateCommandBuffers(device, &alloc_info, command_buffers.data()) != VK_SUCCESS)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create command buffer");
	}
}

void Render_manager::record_command_buffer(VkCommandBuffer command_buffer, uint32_t image_index)
{
	VkCommandBufferBeginInfo begin_info = {};
	begin_info.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	if (vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to begin recording command buffer.");
	}

	VkRenderPassBeginInfo render_pass_info = {};
	render_pass_info.sType                 = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	render_pass_info.renderPass            = render_pass;
	render_pass_info.framebuffer           = swap_chain_frame_buffers[image_index];
	render_pass_info.renderArea.offset     = {0, 0};
	render_pass_info.renderArea.extent     = swap_chain_extent;

	VkClearValue clear_color         = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
	render_pass_info.clearValueCount = 1;
	render_pass_info.pClearValues    = &clear_color;

	vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline);

	VkViewport viewport = {};
	viewport.x          = 0.0f;
	viewport.y          = 0.0f;
	viewport.width      = static_cast<float>(swap_chain_extent.width);
	viewport.height     = static_cast<float>(swap_chain_extent.height);
	viewport.minDepth   = 0.0f;
	viewport.maxDepth   = 1.0f;
	vkCmdSetViewport(command_buffer, 0, 1, &viewport);

	VkRect2D scissors = {};
	scissors.offset   = {0, 0};
	scissors.extent   = swap_chain_extent;
	vkCmdSetScissor(command_buffer, 0, 1, &scissors);

	vkCmdDraw(command_buffer, 3, 1, 0, 0);

	vkCmdEndRenderPass(command_buffer);

	if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to record command buffer.");
	}
}

void Render_manager::create_sync_objects()
{
	image_available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
	render_finished_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
	in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphore_info = {};
	semaphore_info.sType                 = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fence_info = {};
	fence_info.sType             = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_info.flags             = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		if (vkCreateSemaphore(device, &semaphore_info, nullptr, &image_available_semaphores[i]) != VK_SUCCESS
		    || vkCreateSemaphore(device, &semaphore_info, nullptr, &render_finished_semaphores[i]) != VK_SUCCESS || vkCreateFence(device, &fence_info, nullptr, &in_flight_fences[i]) != VK_SUCCESS)
		{
			SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create semaphores.");
		}
	}
}

void Render_manager::draw_frame()
{
	vkWaitForFences(device, 1, &in_flight_fences[current_frame], VK_TRUE, UINT64_MAX);

	uint32_t image_index;
	VkResult result = vkAcquireNextImageKHR(device, swap_chain, UINT64_MAX, image_available_semaphores[current_frame], VK_NULL_HANDLE, &image_index);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebuffer_resized)
	{
		framebuffer_resized = true;
		recreate_swapchain();
	}
	else if (result != VK_SUCCESS)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to acquire swap chain image!");
	}

	vkResetFences(device, 1, &in_flight_fences[current_frame]);

	current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;

	vkResetCommandBuffer(command_buffers[current_frame], 0);
	record_command_buffer(command_buffers[current_frame], image_index);

	VkSubmitInfo submit_info{};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore          waitSemaphores[] = {image_available_semaphores[current_frame]};
	VkPipelineStageFlags waitStages[]     = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	submit_info.waitSemaphoreCount        = 1;
	submit_info.pWaitSemaphores           = waitSemaphores;
	submit_info.pWaitDstStageMask         = waitStages;

	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers    = &command_buffers[current_frame];

	VkSemaphore signalSemaphores[]   = {render_finished_semaphores[current_frame]};
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores    = signalSemaphores;

	if (vkQueueSubmit(graphics_queue, 1, &submit_info, in_flight_fences[current_frame]) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	VkPresentInfoKHR present_info{};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores    = signalSemaphores;

	VkSwapchainKHR swapChains[] = {swap_chain};
	present_info.swapchainCount = 1;
	present_info.pSwapchains    = swapChains;

	present_info.pImageIndices = &image_index;

	vkQueuePresentKHR(present_queue, &present_info);

	current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Render_manager::framebuffer_resize_callback(SDL_Window* window, int width, int height)
{
}
