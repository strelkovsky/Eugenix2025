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
		virtual void OnUpdate(float deltaTime) { }
		virtual void OnRender() { }
		virtual void OnDebugUI() { }
		virtual void OnCleanup() { }

		virtual void OnKeyHandle(int key, int code, int action, int mode) { }
		virtual void OnMouseHandle(double xPos, double yPos) { }
		virtual void OnMouseButtonHandle(int button, int action, int mods) { }

		int Width() const { return _width; }
		int Height() const { return _height; }

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
