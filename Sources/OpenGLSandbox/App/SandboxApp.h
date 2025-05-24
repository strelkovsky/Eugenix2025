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
		SandboxApp(int width = DEFAULT_WIDTH, int height = DEFAULT_HEIGHT);

		int Run();

	protected:
		virtual bool OnInit() { return true; }
		virtual void OnUpdate(float deltaTime) {}
		virtual void OnRender() {}
		virtual void OnCleanup() {}

	private:
		GLFWwindow* _window{ nullptr };
		int _width{};
		int _height{};

		bool initRuntime();
	};
} // namespace Eugenix
