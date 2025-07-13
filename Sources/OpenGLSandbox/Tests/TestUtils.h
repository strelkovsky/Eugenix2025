#pragma once

#include <span>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <stb_image.h>

#include "Render/OpenGL/Commands.h"
#include "Render/OpenGL/ShaderStage.h"
#include "Render/OpenGL/VertexArray.h"

// TMP bridge beetween tests & sandbox sources
namespace Eugenix
{
	class Light
	{
	public:
		Light()
		{
			color = glm::vec3(1.0f, 1.0f, 1.0f);
			ambientIntensity = 1.0f;
		}
		Light(GLfloat red, GLfloat green, GLfloat blue, GLfloat ambIntensity)
		{
			color = glm::vec3(red, green, blue);
			ambientIntensity = ambIntensity;
		}
		void UseLight(GLint ambientColorLocation, GLint ambientIntensityLocation)
		{
			glUniform3f(ambientColorLocation, color.x, color.y, color.z);
			glUniform1f(ambientIntensityLocation, ambientIntensity);
		}
		~Light()
		{

		}

	private:
		glm::vec3 color;
		GLfloat ambientIntensity;
	};

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
		void mouseControl(bool* buttons, GLfloat xChange, GLfloat yChange)
		{
			if (buttons[GLFW_MOUSE_BUTTON_RIGHT])
			{
				yaw += xChange * turnSpeed;
				pitch += yChange * turnSpeed;

				update();
			}
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

	class Texture
	{
	public:
		Texture()
		{
			textureID = 0;
			width = 0;
			height = 0;
			bitDepth = 0;
			fileLocation = "";
		}
		Texture(const char* fileLoc)
			: Texture()
		{
			fileLocation = fileLoc;
		}

		void LoadTexture()
		{
			unsigned char* texData = stbi_load(fileLocation, &width, &height, &bitDepth, 0);
			if (!texData)
			{
				printf("Failed to find: '%s'\n", fileLocation);
				return;
			}

			glGenTextures(1, &textureID);
			glBindTexture(GL_TEXTURE_2D, textureID);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData);
			glGenerateMipmap(GL_TEXTURE_2D);

			glBindTexture(GL_TEXTURE_2D, 0);
			stbi_image_free(texData);
		}

		void UseTexture()
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, textureID);
		}

		void ClearTexture()
		{
			glDeleteTextures(1, &textureID);
			textureID = 0;
			width = 0;
			height = 0;
			bitDepth = 0;
			fileLocation = "";
		}

		~Texture()
		{
			ClearTexture();
		}

	private:
		GLuint textureID;
		int width;
		int height;
		int bitDepth;

		const char* fileLocation;

	};

	Eugenix::Render::OpenGL::ShaderStage CreateStage(const char* source, Eugenix::Render::ShaderStageType type)
	{
		Eugenix::Render::OpenGL::ShaderStage stage{ type };
		stage.Create();
		stage.CompileFromSource(source);
		return stage;
	}
}
