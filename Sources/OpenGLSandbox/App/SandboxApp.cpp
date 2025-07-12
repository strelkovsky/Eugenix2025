#include "SandboxApp.h"

#include "Engine/Core/Time.h"

namespace
{
#if _DEBUG
	void debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, GLchar const* message, void const* user_param)
	{
		auto source_str = [source]() -> std::string {
			switch (source)
			{
			case GL_DEBUG_SOURCE_API: return "API";
			case GL_DEBUG_SOURCE_WINDOW_SYSTEM: return "WINDOW SYSTEM";
			case GL_DEBUG_SOURCE_SHADER_COMPILER: return "SHADER COMPILER";
			case GL_DEBUG_SOURCE_THIRD_PARTY:  return "THIRD PARTY";
			case GL_DEBUG_SOURCE_APPLICATION: return "APPLICATION";
			case GL_DEBUG_SOURCE_OTHER: return "OTHER";
			default: return "UNKNOWN";
			}
			}();

		auto type_str = [type]() {
			switch (type)
			{
			case GL_DEBUG_TYPE_ERROR: return "ERROR";
			case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "DEPRECATED_BEHAVIOR";
			case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: return "UNDEFINED_BEHAVIOR";
			case GL_DEBUG_TYPE_PORTABILITY: return "PORTABILITY";
			case GL_DEBUG_TYPE_PERFORMANCE: return "PERFORMANCE";
			case GL_DEBUG_TYPE_MARKER:  return "MARKER";
			case GL_DEBUG_TYPE_OTHER: return "OTHER";
			default: return "UNKNOWN";
			}
			}();

		auto severity_str = [severity]() {
			switch (severity) {
			case GL_DEBUG_SEVERITY_NOTIFICATION: return "NOTIFICATION";
			case GL_DEBUG_SEVERITY_LOW: return "LOW";
			case GL_DEBUG_SEVERITY_MEDIUM: return "MEDIUM";
			case GL_DEBUG_SEVERITY_HIGH: return "HIGH";
			default: return "UNKNOWN";
			}
			}();

		std::cout << source_str << ", "
			<< type_str << ", "
			<< severity_str << ", "
			<< id << ": "
			<< message << std::endl;
	}
#endif // _DEBUG
}

namespace Eugenix
{
	SandboxApp::SandboxApp(int width, int height)
		: _width(width)
		, _height(height)
	{
	}

	int SandboxApp::Run()
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

		auto lastTime = Time::Clock::now();
		auto fpsTime = lastTime;
		int frameCount = 0;

		while (!glfwWindowShouldClose(_window))
		{
			auto currentTime = Time::Clock::now();
			Time::Duration deltaTime = currentTime - lastTime;
			lastTime = currentTime;

			glfwPollEvents();

			OnUpdate(deltaTime.count());
			OnRender();

			glfwSwapBuffers(_window);

			frameCount++;
			if (Time::Duration(currentTime - fpsTime).count() >= 1.0f)
			{
				float fps = static_cast<float>(frameCount);
				std::ostringstream title;
				title << "EugenixSandbox - FPS: " << std::fixed << std::setprecision(1) << fps
					<< " | Frame Time: " << std::setprecision(3) << (deltaTime.count() * 1000.0f) << " ms";
				glfwSetWindowTitle(_window, title.str().c_str());

				frameCount = 0;
				fpsTime = currentTime;
			}
		}

		OnCleanup();

		return EXIT_SUCCESS;
	}

	bool SandboxApp::initRuntime()
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
			return false;
		}

		glfwSetWindowUserPointer(_window, this);

		for (size_t i = 0; i < 1024; i++)
			_keys[i] = false;

		for (size_t i = 0; i < 32; i++)
			_buttons[i] = false;

		_mouseFirstMoved = true;
		_mouseCursorAboveWindow = false;

		glfwSetKeyCallback(_window, [](GLFWwindow* window, int key, int code, int action, int mode)
			{
				if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
				{
					glfwSetWindowShouldClose(window, true);
				}

				auto* self = static_cast<SandboxApp*>(glfwGetWindowUserPointer(window));
				if (self)
				{
					if (key >= 0 && key < 1024)
					{
						if (action == GLFW_PRESS)
						{
							self->_keys[key] = true;
							//printf("Key pressed: %d\n", key);
						}
						else if (action == GLFW_RELEASE)
						{
							self->_keys[key] = false;
							//printf("Key released: %d\n", key);
						}
					}

					self->OnKeyHandle(key, code, action, mode);
				}
			});

		glfwSetCursorPosCallback(_window, [](GLFWwindow* window, double xpos, double ypos)
			{
				auto* self = static_cast<SandboxApp*>(glfwGetWindowUserPointer(window));
				if (self)
				{
					if (self->_mouseFirstMoved)
					{
						self->_lastX = (GLfloat)xpos;
						self->_lastY = (GLfloat)ypos;
						self->_mouseFirstMoved = false;
					}

					self->_xChange = (GLfloat)xpos - self->_lastX;
					self->_yChange = self->_lastY - (GLfloat)ypos;

					self->_lastX = (GLfloat)xpos;
					self->_lastY = (GLfloat)ypos;

					//printf("x:%.2f, y:%.2f\n", xChange, yChange);

					self->OnMouseHandle(xpos, ypos);
				}
			});

		glfwSetMouseButtonCallback(_window, [](GLFWwindow* window, int button, int action, int mods)
			{
				auto* self = static_cast<SandboxApp*>(glfwGetWindowUserPointer(window));
				if (self)
				{
					if (button >= 0 && button < 32)
					{
						if (action == GLFW_PRESS)
						{
							self->_buttons[button] = true;
							//printf("Mouse button pressed: %d\n", button);
						}
						else if (action == GLFW_RELEASE)
						{
							self->_buttons[button] = false;
							//printf("Mouse button released: %d\n", button);
						}
					}

					self->OnMouseButtonHandle(button, action, mods);
				}
			});

		glfwMakeContextCurrent(_window);

		if (!gladLoadGL())
		{
			std::cout << "Failed to init GLAD" << std::endl;
			glfwTerminate();
		}

#if _DEBUG
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(debugCallback, nullptr);
#endif // _DEBUG

		return true;
	}
} // namespace Eugenix