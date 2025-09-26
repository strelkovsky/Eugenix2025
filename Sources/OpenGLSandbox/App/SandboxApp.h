#pragma once

#include <iostream>
#include <stdexcept>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Render/RenderCaps.h"

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
		virtual bool onInit() { return true; }
		virtual void onUpdate(float deltaTime) { }
		virtual void onRender() { }
		virtual void onDebugUI() { }
		virtual void onCleanup() { }
		virtual void onResize() { }

		virtual void onKeyHandle(int key, int code, int action, int mode) { }
		virtual void onMouseHandle(double xPos, double yPos) { }
		virtual void onMouseButtonHandle(int button, int action, int mods) { }

		int width() const { return _width; }
		int height() const { return _height; }

		GLFWwindow* WindowHandle() const { return _window; }

		bool* getKeys() { return _keys; };
		bool* getMouseButtons() { return _buttons; };

		inline GLfloat getXChange()
		{
			GLfloat theChange = _xChange;
			_xChange = 0.0f;
			return theChange;
		}

		inline GLfloat getYChange()
		{
			GLfloat theChange = _yChange;
			_yChange = 0.0f;
			return theChange;
		}

	private:
		bool initRuntime();

		Render::Caps _renderCaps{};

		GLFWwindow* _window{ nullptr };
		int _width{};
		int _height{};

		bool _keys[1024];
		bool _buttons[32];
		bool _mouseFirstMoved{};
		bool _mouseCursorAboveWindow{};

		GLfloat _lastX{};
		GLfloat _lastY{};
		GLfloat _xChange{};
		GLfloat _yChange{};
	};
} // namespace Eugenix
