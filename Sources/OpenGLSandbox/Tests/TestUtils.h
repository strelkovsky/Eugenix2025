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
	namespace UBO
	{
		struct Camera
		{
			glm::mat4 view;
			glm::mat4 proj;
		};

		struct Material
		{
			glm::vec3 color;
		};
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


	Render::OpenGL::ShaderStage CreateStage(std::string_view source, Render::ShaderStageType type)
	{
		Render::OpenGL::ShaderStage stage{ type };
		stage.Create();
		stage.CompileGLSL(source);
		return stage;
	}

	Render::OpenGL::ShaderStage CreateStage(const std::vector<char>& source, Render::ShaderStageType type)
	{
		Render::OpenGL::ShaderStage stage{ type };
		stage.Create();
		stage.SpecializeSPIRV(source);
		return stage;
	}

	inline Render::OpenGL::Pipeline MakePipeline(std::string_view vsSource, std::string_view fsSource)
	{
		auto vs = CreateStage(vsSource, Render::ShaderStageType::Vertex);
		auto fs = CreateStage(fsSource, Render::ShaderStageType::Fragment);

		Render::OpenGL::Pipeline p;
		p.Create();
		p.AttachStage(vs).AttachStage(fs).Build();

		vs.Destroy();
		fs.Destroy();
		return p;
	}

	inline Render::OpenGL::Pipeline MakePipeline(const std::vector<char>& vsSource, const std::vector<char>& fsSource)
	{
		auto vs = CreateStage(vsSource, Render::ShaderStageType::Vertex);
		auto fs = CreateStage(fsSource, Render::ShaderStageType::Fragment);

		Render::OpenGL::Pipeline p;
		p.Create();
		p.AttachStage(vs).AttachStage(fs).Build();

		vs.Destroy();
		fs.Destroy();
		return p;
	}

	inline Render::OpenGL::Pipeline MakePipelineFromFiles(const std::filesystem::path& vsPath, const std::filesystem::path& fsPath)
	{
		const bool vsSpv = (vsPath.extension() == ".spv");
		const bool fsSpv = (fsPath.extension() == ".spv");
		if (vsSpv != fsSpv)
			throw std::runtime_error("VS/FS format mismatch: both must be .spv or both must be GLSL.");

		if (vsSpv)
		{
			auto vsData = IO::File::ReadBinary(vsPath);
			auto fsData = IO::File::ReadBinary(fsPath);

			return MakePipeline(vsData, fsData);
		}
		else
		{
			auto vsData = IO::File::ReadText(vsPath);
			auto fsData = IO::File::ReadText(fsPath);

			return MakePipeline(
				std::string_view{ vsData.data(), vsData.size() },
				std::string_view{ fsData.data(), fsData.size() }
			);
		}
	}

	// Helpers

	inline int ComponentsFromGLType(GLenum t)
	{
		switch (t) {
		case GL_FLOAT: case GL_INT: case GL_UNSIGNED_INT: case GL_DOUBLE: return 1;
		case GL_FLOAT_VEC2: case GL_INT_VEC2: case GL_UNSIGNED_INT_VEC2: case GL_DOUBLE_VEC2: return 2;
		case GL_FLOAT_VEC3: case GL_INT_VEC3: case GL_UNSIGNED_INT_VEC3: case GL_DOUBLE_VEC3: return 3;
		case GL_FLOAT_VEC4: case GL_INT_VEC4: case GL_UNSIGNED_INT_VEC4: case GL_DOUBLE_VEC4: return 4;
		default:
			throw std::runtime_error("Unsupported GLSL attribute type");
		}
	}


	inline uint32_t BytesPerComponent(Render::DataType dt) 
	{
		switch (dt) {
		case Render::DataType::Float: return 4;
		case Render::DataType::UInt:   return 4;
		default: return 0;
		}
	}

	Render::Attribute AttributeFromShader(const Render::OpenGL::AttribInfo& attribInfo, uint32_t currentOffset = 0)
	{
		Render::Attribute attrib;
		attrib.normalized = false;
		attrib.index = attribInfo.location;

		attrib.type = Render::OpenGL::to_native_type(attribInfo.type) ;
		attrib.size = ComponentsFromGLType(attribInfo.type);

		attrib.offset = currentOffset;

		return attrib;
	}
}
