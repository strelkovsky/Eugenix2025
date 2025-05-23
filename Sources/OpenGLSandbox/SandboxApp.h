#pragma once

#include <iostream>
#include <stdexcept>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

constexpr auto DEFAULT_WIDTH = 1024;
constexpr auto DEFAULT_HEIGHT = 768;

namespace Eugenix
{
	class SandboxApp
	{
	public:
		SandboxApp(int width = DEFAULT_WIDTH, int height = DEFAULT_HEIGHT)
			: _width(width)
			, _height(height)
		{
		}

		int Run()
		{
			if (!initRuntime())
			{
				std::cout << "Failed to init runtime" << std::endl;
				return EXIT_FAILURE;
			}

			if (!OnInit())
			{
				std::cout << "Failed to init client app" << std::endl;
				return EXIT_FAILURE;
			}

			while (!glfwWindowShouldClose(_window))
			{
				glfwPollEvents();

				OnUpdate(0);
				OnRender();

				glfwSwapBuffers(_window);
			}

			OnCleanup();

			return EXIT_SUCCESS;
		}

	protected:
		virtual bool OnInit() { return true; }
		virtual void OnUpdate(float deltaTime) {}
		virtual void OnRender() {}
		virtual void OnCleanup() {}

	private:
		GLFWwindow* _window{ nullptr };
		int _width{};
		int _height{};

		bool initRuntime()
		{
			if (glfwInit() != GLFW_TRUE)
			{
				std::cout << "Failed to init GLFW" << std::endl;
				glfwTerminate();
				return false;
			}

			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

			_window = glfwCreateWindow(_width, _height, "EugenixSandbox", nullptr, nullptr);
			if (_window == nullptr)
			{
				std::cout << "Failed to create GLFW window" << std::endl;
				glfwTerminate();
				return -1;
			}
			glfwMakeContextCurrent(_window);

			if (!gladLoadGL())
			{
				std::cout << "Failed to init GLAD" << std::endl;
				glfwTerminate();
			}

			return true;
		}
	};
} // namespace Eugenix
