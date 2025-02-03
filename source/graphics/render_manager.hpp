#pragma once

#include <optional>
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>


class SDL_Window;
class SDL_Surface;


struct Queue_family_indices
{
	std::optional<uint32_t> graphics_family;
	std::optional<uint32_t> present_family;

	bool is_complete()
	{
		return graphics_family.has_value() && present_family.has_value();
	}
};

struct Swap_chain_support_details
{
	VkSurfaceCapabilitiesKHR        capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR>   present_modes;
};

class Render_manager
{
public:

	bool startup();
	void shutdown();
	void update();

private:

	SDL_Window*                  window = nullptr;
	VkSurfaceKHR                 surface;
	VkInstance                   vulkan_instance;
	VkDebugUtilsMessengerEXT     debug_messenger;
	VkPhysicalDevice             physical_device = VK_NULL_HANDLE;
	VkDevice                     device;
	VkQueue                      graphics_queue;
	VkQueue                      present_queue;
	VkSwapchainKHR               swap_chain;
	std::vector<VkImage>         swap_chain_images;
	std::vector<VkImageView>     swap_chain_image_views;
	VkFormat                     swap_chain_image_format;
	VkExtent2D                   swap_chain_extent;
	VkRenderPass                 render_pass;
	VkPipelineLayout             pipeline_layout;
	VkPipeline                   graphics_pipeline;
	std::vector<VkFramebuffer>   swap_chain_frame_buffers;
	VkCommandPool                command_pool;
	std::vector<VkCommandBuffer> command_buffers;
	std::vector<VkSemaphore>     image_available_semaphores;
	std::vector<VkSemaphore>     render_finished_semaphores;
	std::vector<VkFence>         in_flight_fences;
	uint32_t                     current_frame = 0;
	std::vector<VkFence>         in_flight_fences;
	bool                         framebuffer_resized = false;

	bool create_vulkan_instance();
	void create_surface();
	bool check_validation_layer_support();
	bool check_device_extension_support(VkPhysicalDevice device);

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

	void                 pick_physical_device();
	bool                 is_device_suitable(VkPhysicalDevice device);
	Queue_family_indices find_queue_families(VkPhysicalDevice device);

	void                       create_logical_device();
	Swap_chain_support_details query_swap_chain_support(VkPhysicalDevice device);

	VkSurfaceFormatKHR choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& available_formats);
	VkPresentModeKHR   choose_swap_present_mode(const std::vector<VkPresentModeKHR>& available_present_modes);
	VkExtent2D         choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities);
	void               create_swapchain();
	void               recreate_swapchain();
	void               cleanup_swapchain();
	void               create_image_views();
	void               create_graphics_pipeline();

	static std::vector<char> read_file(const std::string& filename);
	VkShaderModule           create_shader_module(const std::vector<char>& code);
	void                     create_render_pass();
	void                     create_frame_buffers();
	void                     create_command_pool();
	void                     create_command_buffers();
	void                     record_command_buffer(VkCommandBuffer command_buffer, uint32_t image_index);
	void                     create_sync_objects();
	void                     draw_frame();
	static void              framebuffer_resize_callback(SDL_Window* window, int width, int height);
};