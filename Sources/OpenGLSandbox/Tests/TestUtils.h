#pragma once

#include <span>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <stb_image.h>

#include "Engine/IO/IO.h"

#include "Render/OpenGL/Commands.h"
#include "Render/OpenGL/Pipeline.h"
#include "Render/OpenGL/ShaderStage.h"
#include "Render/OpenGL/VertexArray.h"

const int MAX_POINT_LIGHTS = 3;
const int MAX_SPOT_LIGHTS = 3;

// TMP bridge beetween tests & sandbox sources
namespace Eugenix
{
	class Light
	{
	public:
		Light()
		{
			m_Color = glm::vec3(1.0f, 1.0f, 1.0f);
			m_AmbientIntensity = 1.0f;
			m_DiffuseIntensity = 0.0f;
		}
		Light(glm::vec3 color, GLfloat ambientIntensity, GLfloat diffuseIntensity)
		{
			m_Color = glm::vec3(color.r, color.g, color.b);
			m_AmbientIntensity = ambientIntensity;
			m_DiffuseIntensity = diffuseIntensity;
		}

		~Light()
		{

		}

	protected:
		glm::vec3 m_Color;
		GLfloat m_AmbientIntensity;
		GLfloat m_DiffuseIntensity;
	};

	class DirectionalLight : public Light
	{

	public:
		DirectionalLight()
			: Light()
		{
			m_Direction = glm::vec3(0.0f, -1.0f, 0.0f);
		}

		DirectionalLight(glm::vec3 color, GLfloat ambientIntensity, GLfloat diffuseIntensity, glm::vec3 direction)
			: Light(color, ambientIntensity, diffuseIntensity)
		{
			m_Direction = glm::normalize(direction);
		}

		void UseLight(GLint ambientColorLocation, GLint ambientIntensityLocation,
			GLint diffuseIntensityLocation, GLint directionLocation)
		{
			glUniform3f(ambientColorLocation, m_Color.x, m_Color.y, m_Color.z);
			glUniform1f(ambientIntensityLocation, m_AmbientIntensity);
			glUniform1f(diffuseIntensityLocation, m_DiffuseIntensity);
			glUniform3f(directionLocation, m_Direction.x, m_Direction.y, m_Direction.z);
		}

		~DirectionalLight() { }

	protected:
		glm::vec3 m_Direction;
	};

	class PointLight : public Light
	{
	public:
		PointLight()
			: Light()
		{
			m_Position = glm::vec3(0.0f, 0.0f, 0.0f);
			m_Constant = 1.0f;
			m_Linear = 0.0f;
			m_Exponent = 0.0f;
		}

		PointLight(glm::vec3 color, GLfloat ambientIntensity, GLfloat diffuseIntensity,
			glm::vec3 position, GLfloat constant, GLfloat linear, GLfloat exponent)
			: Light(color, ambientIntensity, diffuseIntensity)
		{
			m_Position = position;
			m_Constant = constant;
			m_Linear = linear;
			m_Exponent = exponent;
		}

		void UseLight(GLint ambientColorLocation, GLint ambientIntensityLocation, GLint diffuseIntensityLocation,
			GLint positionLocation, GLint constantLocation, GLint linearLocation, GLint exponentLocation)
		{
			glUniform3f(ambientColorLocation, m_Color.r, m_Color.g, m_Color.b);
			glUniform1f(ambientIntensityLocation, m_AmbientIntensity);
			glUniform1f(diffuseIntensityLocation, m_DiffuseIntensity);
			glUniform3f(positionLocation, m_Position.x, m_Position.y, m_Position.z);
			glUniform1f(constantLocation, m_Constant);
			glUniform1f(linearLocation, m_Linear);
			glUniform1f(exponentLocation, m_Exponent);
		}

		~PointLight() { }

	protected:
		glm::vec3 m_Position;
		GLfloat m_Constant;
		GLfloat m_Linear;
		GLfloat m_Exponent;
	};


	class SpotLight : public DirectionalLight, PointLight
	{
	public:
		SpotLight() : DirectionalLight(), PointLight()
		{

		}

		SpotLight(glm::vec3 color, GLfloat ambientIntensity, GLfloat diffuseIntensity,
			glm::vec3 position, glm::vec3 direction,
			GLfloat constant, GLfloat linear, GLfloat exponent, GLfloat edge)
			: DirectionalLight(color, ambientIntensity, diffuseIntensity, direction),
			PointLight(color, ambientIntensity, diffuseIntensity, position, constant, linear, exponent)
		{
			m_Edge = edge;
			m_EdgeProcessed = cosf(glm::radians(m_Edge));
		}

		void UseLight(GLint ambientColorLocation, GLint ambientIntensityLocation, GLint diffuseIntensityLocation,
			GLint positionLocation, GLint directionLocation,
			GLint constantLocation, GLint linearLocation, GLint exponentLocation,
			GLint edgeLocation)
		{
			glUniform3f(ambientColorLocation, Light::m_Color.r, Light::m_Color.g, Light::m_Color.b);
			glUniform1f(ambientIntensityLocation, Light::m_AmbientIntensity);
			glUniform1f(diffuseIntensityLocation, Light::m_DiffuseIntensity);
			glUniform3f(positionLocation, m_Position.x, m_Position.y, m_Position.z);
			glUniform3f(directionLocation, m_Direction.x, m_Direction.y, m_Direction.z);
			glUniform1f(constantLocation, m_Constant);
			glUniform1f(linearLocation, m_Linear);
			glUniform1f(exponentLocation, m_Exponent);
			glUniform1f(edgeLocation, m_EdgeProcessed);
		}

		void SetFlash(glm::vec3 position, glm::vec3 direction)
		{
			m_Position = position;
			m_Direction = direction;
		}

		~SpotLight()
		{

		}

	private:
		GLfloat m_Edge;
		GLfloat m_EdgeProcessed;

	};

	struct SimpleMesh
	{
		// TODO : vertexSize - плохое решение. Возможно передавать Vertex в шаблонном параметре.
		SimpleMesh(std::span<const float> vertices, std::span<const uint32_t> indices, std::span<const Render::Attribute> attributes, float vertexSize)
			: indexCount{ static_cast<uint32_t>(indices.size()) }
		{
			_vao.Create();

			_vbo.Create();
			_vbo.Storage(Core::MakeData(vertices));

			_ibo.Create();
			_ibo.Storage(Core::MakeData(indices));

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
			Render::OpenGL::Commands::DrawIndexed(Render::PrimitiveType::Triangles, indexCount, Render::DataType::UInt);
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
			m_Position = startPosition;
			m_WorldUp = startUp;
			m_Yaw = startYaw;
			m_Pitch = startPitch;
			m_Front = glm::vec3(0.0f, 0.0f, -1.0f);

			m_MoveSpeed = startMoveSpeed;
			m_TurnSpeed = startTurnSpeed;

			update();
		}

		void keyControl(bool* keys, GLfloat deltaTime)
		{
			GLfloat velocity = m_MoveSpeed * deltaTime;

			if (keys[GLFW_KEY_LEFT_SHIFT])
			{
				velocity *= m_SpeedBoost;
			}

			if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP])
			{
				m_Position += m_Front * velocity;
			}
			if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN])
			{
				m_Position -= m_Front * velocity;
			}
			if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT])
			{
				m_Position -= m_Right * velocity;
			}
			if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT])
			{
				m_Position += m_Right * velocity;
			}
			if (keys[GLFW_KEY_Q])
			{
				m_Position -= m_Up * velocity;
			}
			if (keys[GLFW_KEY_E] || keys[GLFW_KEY_SPACE])
			{
				m_Position += m_Up * velocity;
			}

			if (keys[GLFW_KEY_L])
			{
				printf("Camera Position X: %.2f Y: %.2f Z: %.2f\n", m_Position.x, m_Position.y, m_Position.z);
				printf("Camera Direction X: %.2f Y: %.2f Z: %.2f\n", m_Front.x, m_Front.y, m_Front.z);
			}
		}
		void mouseControl(bool* buttons, GLfloat xChange, GLfloat yChange)
		{
			if (buttons[GLFW_MOUSE_BUTTON_RIGHT])
			{
				m_Yaw += xChange * m_TurnSpeed;
				m_Pitch += yChange * m_TurnSpeed;

				update();
			}
		}

		glm::vec3 getCameraPosition()
		{
			return m_Position;
		}

		glm::vec3 getCameraDirection()
		{
			return glm::normalize(m_Front);
		}

		glm::mat4 CalculateViewMatrix()
		{
			glm::mat4 viewMatrix = glm::lookAt(m_Position, m_Position + m_Front, m_Up);
			return viewMatrix;
		}

		~Camera() { }

	private:
		void update()
		{
			//printf("Pitch: %.2f, Yaw: %.2f\n", pitch, yaw);

			m_Front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
			m_Front.y = sin(glm::radians(m_Pitch));
			m_Front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
			m_Front = glm::normalize(m_Front);

			m_Right = glm::normalize(glm::cross(m_Front, m_WorldUp));
			m_Up = glm::normalize(glm::cross(m_Right, m_Front));
		}

	private:
		glm::vec3 m_Position;
		glm::vec3 m_Front;
		glm::vec3 m_Up;
		glm::vec3 m_Right;
		glm::vec3 m_WorldUp;

		GLfloat m_Yaw = 0.0f;
		GLfloat m_Pitch = 0.0f;
		GLfloat m_Roll = 0.0f; // not used

		GLfloat m_MoveSpeed;
		GLfloat m_TurnSpeed;

		GLfloat m_SpeedBoost = 4.0f;
	};

	class ShadowMap
	{
	public:

		ShadowMap()
		{
			FBO = 0;
			shadowMap = 0;
		}

		virtual bool Init(GLuint width, GLuint height)
		{
			shadowWidth = width;
			shadowHeight = height;

			glGenFramebuffers(1, &FBO);
			glGenTextures(1, &shadowMap);
			glBindTexture(GL_TEXTURE_2D, shadowMap);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowWidth, shadowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glBindFramebuffer(GL_FRAMEBUFFER, FBO);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap, 0);

			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);

			GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

			if (status != GL_FRAMEBUFFER_COMPLETE)
			{
				printf("Framebuffer Error: %i\n", status);
				return false;
			}

			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			return true;
		}

		virtual void Write()
		{

		}

		virtual void Read(GLenum textureUnit)
		{

		}

		GLuint GetShadowWidth() { return shadowWidth; };
		GLuint GetShadowHeight() { return shadowHeight; };
		~ShadowMap()
		{

		}

	protected:

		GLuint FBO;
		GLuint shadowMap;
		GLuint shadowWidth;
		GLuint shadowHeight;


	};

	Render::OpenGL::ShaderStage CreateStage(const char* source, Render::ShaderStageType type)
	{
		Render::OpenGL::ShaderStage stage{ type };
		stage.Create();
		stage.CompileFromSource(source);
		return stage;
	}

	Render::OpenGL::Pipeline MakePipeline(std::string_view vsPath, std::string_view fsPath)
	{
		auto vsData = IO::FileContent(vsPath);
		auto fsData = IO::FileContent(fsPath);

		auto vs = CreateStage(vsData.data(), Render::ShaderStageType::Vertex);
		auto fs = CreateStage(fsData.data(), Render::ShaderStageType::Fragment);

		Render::OpenGL::Pipeline p;
		p.Create();
		p.AttachStage(vs).AttachStage(fs).Build();

		vs.Destroy();
		fs.Destroy();
		return p;
	}
}
