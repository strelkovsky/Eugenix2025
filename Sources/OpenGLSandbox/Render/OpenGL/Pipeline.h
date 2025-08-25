#pragma once

#include <algorithm>

#include <glad/glad.h>

#include <glm/gtc/type_ptr.hpp>

#include "Object.h"
#include "ShaderStage.h"

namespace Eugenix::Render::OpenGL
{
	struct AttribInfo
	{
		GLenum type;
		std::string name;
		GLint location;
	};

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

			processAttributes();

			return *this;
		}

		void Bind()
		{
			glUseProgram(_handle);
		}

		// TODO : cache locations

		// ------------------------------------------------------------------------
		void setUniformBool(const std::string& name, bool value) const
		{
			auto location = glGetUniformLocation(_handle, name.data());
			glProgramUniform1i(_handle, location, (int)value);
		}
		// ------------------------------------------------------------------------
		void SetUniform(std::string_view name, int value)
		{
			auto location = glGetUniformLocation(_handle, name.data());
			glProgramUniform1i(_handle, location, value);
		}
		// ------------------------------------------------------------------------
		void SetUniform(std::string_view name, float value) const
		{
			auto location = glGetUniformLocation(_handle, name.data());
			glProgramUniform1f(_handle, location, value);
		}
		// ------------------------------------------------------------------------
		void SetUniform(std::string_view name, const glm::vec2& value) const
		{
			auto location = glGetUniformLocation(_handle, name.data());
			glProgramUniform2fv(_handle, location, 1, &value[0]);
		}
		// ------------------------------------------------------------------------
		void SetUniform(std::string_view name, const glm::vec3& value) const
		{
			auto location = glGetUniformLocation(_handle, name.data());
			glProgramUniform3fv(_handle, location, 1, &value[0]);
		}
		// ------------------------------------------------------------------------
		void SetUniform(std::string_view name, const glm::vec4& value) const
		{
			auto location = glGetUniformLocation(_handle, name.data());
			glProgramUniform4fv(_handle, location, 1, &value[0]);
		}
		// ------------------------------------------------------------------------
		void SetUniform(std::string_view name, const glm::mat3& mat) const
		{
			auto location = glGetUniformLocation(_handle, name.data());
			glProgramUniformMatrix3fv(_handle, location, 1, GL_FALSE, glm::value_ptr(mat));
		}
		// ------------------------------------------------------------------------
		void SetUniform(std::string_view name, const glm::mat4& mat) const
		{
			auto location = glGetUniformLocation(_handle, name.data());
			glProgramUniformMatrix4fv(_handle, location, 1, GL_FALSE, glm::value_ptr(mat));
		}

		const std::vector<AttribInfo>& GetAttribs() const
		{
			return _attribs;
		}
	private:
		std::vector<AttribInfo> _attribs;

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

				LogError("Pipeline link error - {}", programLog.data());
			}
		}

		void processAttributes()
		{
			int activeAttribs = 0;
			glGetProgramiv(_handle, GL_ACTIVE_ATTRIBUTES, &activeAttribs);
			int maxLength = 0;
			glGetProgramiv(_handle, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxLength);

			LogInfo("shader active attribs - {}, maxLength - {}", activeAttribs, maxLength);

			GLint inputs = 0, maxNameLen = 0;
			glGetProgramInterfaceiv(_handle, GL_PROGRAM_INPUT, GL_ACTIVE_RESOURCES, &inputs);
			glGetProgramInterfaceiv(_handle, GL_PROGRAM_INPUT, GL_MAX_NAME_LENGTH, &maxNameLen);

			std::vector<char> name(maxNameLen);
			const GLenum props[] = { GL_NAME_LENGTH, GL_TYPE, GL_LOCATION, GL_ARRAY_SIZE };
			for (GLint i = 0; i < inputs; ++i) {
				GLint vals[4];
				glGetProgramResourceiv(_handle, GL_PROGRAM_INPUT, i, 4, props, 4, nullptr, vals);

				GLsizei nlen = static_cast<GLsizei>(vals[0]);
				GLsizei outLen = 0;
				name.resize(nlen);
				glGetProgramResourceName(_handle, GL_PROGRAM_INPUT, i, nlen, &outLen, name.data());

				std::string attribName(name.data(), outLen);
				GLenum type = static_cast<GLenum>(vals[1]);
				GLint  loc = vals[2];

				if (loc >= 0) 
					_attribs.push_back({ type, std::move(attribName), loc });
			}
			std::sort(_attribs.begin(), _attribs.end(), [](auto& a, auto& b) { return a.location < b.location; });
		}
	};
} // namespace Eugenix::Render::OpenGL
