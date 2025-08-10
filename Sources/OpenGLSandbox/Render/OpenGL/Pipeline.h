#pragma once

#include <glad/glad.h>

#include "Object.h"
#include "ShaderStage.h"

namespace Eugenix::Render::OpenGL
{
	class Pipeline final : public Object
	{
	public:
		void Create() override
		{
			_handle = glCreateProgram();
		}

		void Destroy() override
		{
			glDeleteProgram(_handle);
		}

		Pipeline& AttachStage(ShaderStage& stage)
		{
			glAttachShader(_handle, stage.NativeHandle());
			return *this;
		}
			
		Pipeline& Build()
		{
			glLinkProgram(_handle);
			checkLinkStatus();
			return *this;
		}

		void Bind()
		{
			glUseProgram(_handle);
		}

		void SetUniform(std::string_view name, glm::vec3& color)
		{
			auto location = glGetUniformLocation(_handle, name.data());
			glUniform4f(location, color.x, color.y, color.z, 1.0f);
		}

		void SetUniform(std::string_view name, int value)
		{
			auto location = glGetUniformLocation(_handle, name.data());
			glUniform1i(location, value);
		}

	private:
		void checkLinkStatus()
		{
			GLint success;
			glGetProgramiv(_handle, GL_LINK_STATUS, &success);
			if (!success)
			{
				GLint logLength;
				glGetProgramiv(_handle, GL_INFO_LOG_LENGTH, &logLength);
				std::vector<char> programLog(logLength);
				glGetProgramInfoLog(_handle, logLength, nullptr, programLog.data());

				LogError("Pipeline link error - ", programLog.data());
			}
		}
	};
} // namespace Eugenix::Render::OpenGL
