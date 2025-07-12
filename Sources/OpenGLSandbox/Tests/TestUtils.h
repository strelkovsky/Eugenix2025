#pragma once

#include <span>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Render/OpenGL/Commands.h"
#include "Render/OpenGL/ShaderStage.h"
#include "Render/OpenGL/VertexArray.h"

// TMP bridge beetween tests & sandbox sources
namespace Eugenix
{
	struct SimpleMesh
	{
		// TODO : vertexSize - плохое решение. Возможно передавать Vertex в шаблонном параметре.
		SimpleMesh(std::span<const float> vertices, std::span<const uint32_t> indices, std::span<const Render::Attribute> attributes, float vertexSize)
			: indexCount{ static_cast<uint32_t>(indices.size()) }
		{
			_vao.Create();

			_vbo.Create();
			_vbo.Storage(Eugenix::Core::MakeData(vertices));

			_ibo.Create();
			_ibo.Storage(Eugenix::Core::MakeData(indices));

			_vao.AttachVertices(_vbo, vertexSize);
			_vao.AttachIndices(_ibo);

			for (const auto& attribute : attributes)
			{
				_vao.Attribute(attribute);
			}
		}

		void RenderMesh()
		{
			_vao.Bind();
			Render::OpenGL::Commands::DrawIndexed(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT);
		}

		void ClearMesh()
		{
			_vbo.Destroy();
			_ibo.Destroy();
			_vao.Destroy();

			indexCount = 0;
		}

		~SimpleMesh()
		{
			ClearMesh();
		}

		uint32_t indexCount = 0;

		Render::OpenGL::VertexArray _vao{};
		Render::OpenGL::Buffer _vbo{};
		Render::OpenGL::Buffer _ibo{};
	};

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

	Eugenix::Render::OpenGL::ShaderStage CreateStage(const char* source, Eugenix::Render::ShaderStageType type)
	{
		Eugenix::Render::OpenGL::ShaderStage stage{ type };
		stage.Create();
		stage.CompileFromSource(source);
		return stage;
	}
}
