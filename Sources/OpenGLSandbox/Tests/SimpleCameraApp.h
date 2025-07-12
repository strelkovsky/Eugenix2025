#pragma once

#include <span>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "TestUtils.h"

// Sandbox headers
#include "App/SandboxApp.h"
#include "Render/OpenGL/Buffer.h"
#include "Render/OpenGL/Commands.h"
#include "Render/OpenGL/Pipeline.h"
#include "Render/OpenGL/VertexArray.h"

namespace
{
	bool keys[1024];
	bool buttons[32];

	GLfloat lastX;
	GLfloat lastY;
	GLfloat xChange;
	GLfloat yChange;
	bool mouseFirstMoved;
	bool mouseCursorAboveWindow;

	inline GLfloat getXChange()
	{
		GLfloat theChange = xChange;
		xChange = 0.0f;
		return theChange;
	}

	inline GLfloat getYChange()
	{
		GLfloat theChange = yChange;
		yChange = 0.0f;
		return theChange;
	}

	inline void handleKeys(GLFWwindow* window, int key, int code, int action, int mode)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}

		if (key >= 0 && key < 1024)
		{
			if (action == GLFW_PRESS)
			{
				keys[key] = true;
				//printf("Key pressed: %d\n", key);
			}
			else if (action == GLFW_RELEASE)
			{
				keys[key] = false;
				//printf("Key released: %d\n", key);
			}
		}
	}

	inline void handleMouse(GLFWwindow* window, double xPos, double yPos)
	{
		if (mouseFirstMoved)
		{
			lastX = (GLfloat)xPos;
			lastY = (GLfloat)yPos;
			mouseFirstMoved = false;
		}

		xChange = (GLfloat)xPos - lastX;
		yChange = lastY - (GLfloat)yPos;

		lastX = (GLfloat)xPos;
		lastY = (GLfloat)yPos;

		//printf("x:%.2f, y:%.2f\n", xChange, yChange);
	}

	inline void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
	{
		if (button >= 0 && button < 32)
		{
			if (action == GLFW_PRESS)
			{
				buttons[button] = true;
				printf("Mouse button pressed: %d\n", button);
			}
			else if (action == GLFW_RELEASE)
			{
				buttons[button] = false;
				printf("Mouse button released: %d\n", button);
			}
		}

		if (buttons[GLFW_MOUSE_BUTTON_LEFT] && mouseCursorAboveWindow)
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}
	}

	inline void cursorEnterCallback(GLFWwindow* window, int entered)
	{
		if (entered)
			mouseCursorAboveWindow = true;
		else
			mouseCursorAboveWindow = false;
	}


	class Camera
	{

	public:
		Camera()
			: Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 0.0f, 0.0f, 2.0f, 0.1f)
		{
		}
		Camera(glm::vec3 startPosition, glm::vec3 startUp, GLfloat startYaw, GLfloat startPitch,
			GLfloat startMoveSpeed, GLfloat startTurnSpeed)
		{
			position = startPosition;
			worldUp = startUp;
			yaw = startYaw;
			pitch = startPitch;
			front = glm::vec3(0.0f, 0.0f, -1.0f);

			moveSpeed = startMoveSpeed;
			turnSpeed = startTurnSpeed;

			update();
		}
		void keyControl(bool* keys, GLfloat deltaTime)
		{
			GLfloat velocity = moveSpeed * deltaTime;

			if (keys[GLFW_KEY_W])
			{
				position += front * velocity;
			}
			if (keys[GLFW_KEY_S])
			{
				position -= front * velocity;
			}
			if (keys[GLFW_KEY_A])
			{
				position -= right * velocity;
			}
			if (keys[GLFW_KEY_D])
			{
				position += right * velocity;
			}
			if (keys[GLFW_KEY_Q])
			{
				position -= up * velocity;
			}
			if (keys[GLFW_KEY_E])
			{
				position += up * velocity;
			}
		}
		void mouseControl(GLfloat xChange, GLfloat yChange)
		{
			yaw += xChange * turnSpeed;
			pitch += yChange * turnSpeed;

			update();
		}
		glm::mat4 CalculateViewMatrix()
		{
			glm::mat4 viewMatrix = glm::lookAt(position, position + front, up);
			return viewMatrix;
		}

		~Camera() { }

	private:
		void update()
		{
			//printf("Pitch: %.2f, Yaw: %.2f\n", pitch, yaw);

			front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
			front.y = sin(glm::radians(pitch));
			front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
			front = glm::normalize(front);

			right = glm::normalize(glm::cross(front, worldUp));
			up = glm::normalize(glm::cross(right, front));
		}

	private:
		glm::vec3 position;
		glm::vec3 front;
		glm::vec3 up;
		glm::vec3 right;
		glm::vec3 worldUp;

		GLfloat yaw = 0.0f;
		GLfloat pitch = 0.0f;
		GLfloat roll = 0.0f; // not used

		GLfloat moveSpeed;
		GLfloat turnSpeed;

	};
}

Camera camera;

GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;

namespace Eugenix
{
	class SimpleCameraApp final : public SandboxApp
	{
	protected:
		bool OnInit() override
		{
			CreateGeometry();
			CreatePipelines();

			glEnable(GL_DEPTH_TEST);

			Render::OpenGL::Commands::Clear(0.2f, 0.0f, 0.2f);

			for (size_t i = 0; i < 1024; i++)
				keys[i] = false;

			for (size_t i = 0; i < 32; i++)
				buttons[i] = false;

			mouseFirstMoved = true;
			mouseCursorAboveWindow = false;

			SetKeyCallback(handleKeys);
			SetCursorPosCallback(handleMouse);
			SetCursorEnterCallback(cursorEnterCallback);
			SetMouseButtonCallback(mouseButtonCallback);
		
			camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f, 2.0f, 0.1f);

			return true;
		}

		void OnUpdate(float delteTime) override
		{
			camera.keyControl(keys, deltaTime);
			camera.mouseControl(getXChange(), getYChange());
		}

		void OnRender() override
		{
			Render::OpenGL::Commands::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Projection matrix
			glm::mat4 projection = glm::perspective(45.0f, (GLfloat)Width() / (GLfloat)Height(), 0.1f, 100.0f);
			_pipeline.Bind();

			{
				// Model matrix
				glm::mat4 model = glm::mat4(1.0f);

				model = glm::mat4(1.0f);
				model = glm::translate(model, glm::vec3(-0.6f, 0.0f, -3.0f));
				model = glm::rotate(model, 0.0f, glm::vec3(1.0f, 0.0f, 0.0f));
				model = glm::rotate(model, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
				model = glm::rotate(model, 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
				model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.75f));
				glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
				glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.CalculateViewMatrix()));
				glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
				meshes[0].RenderMesh();

				model = glm::mat4(1.0f);
				model = glm::translate(model, glm::vec3(0.6f, 0.0f, -3.0f));
				model = glm::rotate(model, 0.0f, glm::vec3(1.0f, 0.0f, 0.0f));
				model = glm::rotate(model, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
				model = glm::rotate(model, 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
				model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.75f));
				glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
				glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.CalculateViewMatrix()));
				glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
				meshes[1].RenderMesh();
			}
		}

	private:
		void CreateGeometry()
		{
			GLfloat vertices[] =
			{
				-1.0f, -1.0f, 0.0f,
				 0.0f, -1.0f, 1.0f,
				 1.0f, -1.0f, 0.0f,
				 0.0f,  1.0f, 0.0f,
			};

			unsigned int indices[] =
			{
				0, 3, 1,
				1, 3, 2,
				2, 3, 0,
				0, 1, 2,
			};

			meshes.reserve(2);

			meshes.emplace_back(vertices, indices);
			meshes.emplace_back(vertices, indices);
		}

		void CreatePipelines()
		{
			_pipeline.Create();

			const auto vsSourceData = Eugenix::IO::FileContent("Shaders/shader.vert");
			const char* vsSource = vsSourceData.data();

			const auto fsSourceData = Eugenix::IO::FileContent("Shaders/shader.frag");
			const char* fsSource = fsSourceData.data();

			auto vertexStage = Eugenix::CreateStage(vsSource, Eugenix::Render::ShaderStageType::Vertex);
			auto fragmentStage = Eugenix::CreateStage(fsSource, Eugenix::Render::ShaderStageType::Fragment);

			_pipeline.Create();
			_pipeline.AttachStage(vertexStage)
				.AttachStage(fragmentStage)
				.Build();

			uniformModel = glGetUniformLocation(_pipeline.NativeHandle(), "model");
			uniformView = glGetUniformLocation(_pipeline.NativeHandle(), "view");
			uniformProjection = glGetUniformLocation(_pipeline.NativeHandle(), "projection");
		}

		std::vector<SimpleMesh> meshes;

		Eugenix::Render::OpenGL::Pipeline _pipeline{};
		GLuint uniformModel{};
		GLuint uniformView{};
		GLuint uniformProjection{};
	};
}
