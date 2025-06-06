#pragma once

#include <sstream>

#include "Core/Log.h"

#include "VulkanAdapter.h"
#include "VulkanDevice.h"
#include "VulkanInstance.h"
#include "VulkanSurface.h"
#include "VulkanSwapchain.h"

constexpr auto DEFAULT_WIDTH = 1024;
constexpr auto DEFAULT_HEIGHT = 768;

namespace Eugenix
{
	namespace Render
	{
		namespace Vulkan
		{
			struct VulkanAppConfig
			{
				uint32_t apiVersion         { VK_API_VERSION_1_4 };
				bool enableValidationLayers { true };

				uint32_t windowWidth        { DEFAULT_WIDTH };
				uint32_t windowHeight       { DEFAULT_HEIGHT };
			};

			class VulkanApp
			{
			public:
				int Run(const VulkanAppConfig& config)
				{
					if (!initWindow(config))
					{
						LogError("Failed to create app window!");
						return -1;
					}

					if (!initVulkan(config, _window))
					{
						LogError("Failed to create app window!");
						return -1;
					}

					if (!onInit())
					{
						LogError("App onInit failed!");
						return -1;
					}

					_lastTime = glfwGetTime();

					while (!glfwWindowShouldClose(_window))
					{
						glfwPollEvents();

						if (glfwGetKey(_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
						{
							glfwSetWindowShouldClose(_window, 1);
						}

						auto currentTime = glfwGetTime();
						auto deltaTime = currentTime - _lastTime;
						_lastTime = currentTime;

						// TODO : move stats into imgui
						updateFPS();

						onUpdate(deltaTime);
						onRender();
						onRenderUI();
					}

					vkDeviceWaitIdle(_device.Handle());

					onCleanup();
					cleanupVulkan();

					return 0;
				}

			protected:
				virtual bool onInit() { return true; }
				virtual void onUpdate(float) {}
				virtual void onRender() {}
				virtual void onRenderUI() {}
				virtual void onCleanup() {}

				virtual void onCursorMove(double x, double y) {}

				bool KeyPress(int key)
				{
					return (glfwGetKey(_window, key) == GLFW_PRESS);
				}

				GLFWwindow* _window{ nullptr };

				Instance _instance;
				Surface _surface;
				Adapter _adapter;
				Device _device;
				Swapchain _swapchain;

				bool _resized{ false };

			private:
				double _lastTime{};
				double _lastFPSTime{};
				int _frameCounter{};

				bool initWindow(const VulkanAppConfig& config)
				{
					glfwInit();
					glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
					glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

					_window = glfwCreateWindow(config.windowWidth, config.windowHeight, "Eugenix", nullptr, nullptr);

					glfwSetWindowUserPointer(_window, this);

					glfwSetFramebufferSizeCallback(_window, 
						[](GLFWwindow* window, int width, int height)
						{
							auto* app = static_cast<VulkanApp*>(glfwGetWindowUserPointer(window));
							if (app)
							{
								app->_resized = true;
							}
						});
					glfwSetCursorPosCallback(_window,
						[](GLFWwindow* window, double xpos, double ypos) 
						{
							auto* app = static_cast<VulkanApp*>(glfwGetWindowUserPointer(window));
							if (app) 
							{
								app->onCursorMove(xpos, ypos);
							}
						}
					);

					LogSuccess("App Window created successfully");
					return true;
				}

				bool initVulkan(const VulkanAppConfig& config, GLFWwindow* window)
				{
					if (!_instance.Create(config.apiVersion, config.enableValidationLayers))
						return false;

					if (!_surface.Create(_instance.Handle(), window))
						return false;

					if (!_adapter.Select(_instance.Handle(), _surface.Handle()))
						return false;

					if (!_device.Create(_adapter))
						return false;

					if (!_swapchain.Create(_adapter, _surface, _device, window))
						return false;

					return true;
				}

				void cleanupVulkan()
				{
					_swapchain.Destroy(_device.Handle());
					_device.Destroy();
					_surface.Destroy(_instance.Handle());
					_instance.Destroy();
				}

				void updateFPS()
				{
					double currentTime = glfwGetTime();
					double delta = currentTime - _lastFPSTime;

					_frameCounter++;
					if (delta >= 1.0) // if last update was more than one second ago
					{
						double fps = double(_frameCounter) / delta;

						std::stringstream ss;
						ss << "Eugenix. FPS: " << fps;

						glfwSetWindowTitle(_window, ss.str().c_str());

						_frameCounter = 0;

						_lastFPSTime = currentTime;
					}
				}

			};
		}
	} // namespace Render
} // namespace Eugenix
