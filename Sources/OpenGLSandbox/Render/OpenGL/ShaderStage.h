#pragma once

#include <vector>

#include "SandboxCompileConfig.h"

#include "Engine/Core/Log.h"

#include "Object.h"
#include "OpenGLTypes.h"

namespace Eugenix::Render::OpenGL
{
	class ShaderStage final : public Object
	{
	public:

		// TODO : move it in Object
		//ShaderStage(const ShaderStage&) = delete;
		//ShaderStage& operator=(const ShaderStage&) = delete;

		//ShaderStage(ShaderStage&& other) noexcept
		//	: Object()               
		//	, _stageType(other._stageType)
		//{
		//	_handle = other._handle;
		//	_extra = other._extra;
		//	other._handle = 0;
		//	other._extra = 0;
		//}

		//ShaderStage& operator=(ShaderStage&& other) noexcept
		//{
		//	if (this != &other)
		//	{
		//		_handle = other._handle;
		//		_extra = other._extra;
		//		_stageType = other._stageType;

		//		other._handle = 0;
		//		other._extra = 0;
		//	}
		//	return *this;
		//}

		ShaderStage(ShaderStageType type)
			: _stageType(type)
		{

		}

		void Create() override
		{
			_handle = glCreateShader(to_opengl_type(_stageType));
		}

		void Destroy() override
		{
			glDeleteShader(_handle);
		}

		void CompileGLSL(std::string_view source)
		{
			const char* p = source.data();
			const GLint  n = static_cast<GLint>(source.size());
			glShaderSource(_handle, 1, &p, &n);
			glCompileShader(_handle);
			checkCompileStatus();
		}

		void SpecializeSPIRV(const std::vector<char>& source, const char* entry = "main")
		{
			if (source.size() % 4 != 0)
				throw std::runtime_error("SPIR-V size must be multiple of 4 bytes");

			glShaderBinary(1, &_handle, GL_SHADER_BINARY_FORMAT_SPIR_V, source.data(), static_cast<GLsizei>(source.size()));
			glSpecializeShader(_handle, entry, 0, nullptr, nullptr);

			checkCompileStatus();
		}

	private:
		void checkCompileStatus()
		{
			GLint success;
			glGetShaderiv(_handle, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				GLint logLength;
				glGetShaderiv(_handle, GL_INFO_LOG_LENGTH, &logLength);
				std::vector<char> shaderLog(logLength);
				glGetShaderInfoLog(_handle, logLength, nullptr, shaderLog.data());

				LogError("Shader compile error - {}", shaderLog.data());
			}
		}

		ShaderStageType _stageType{};
	};
} // namespace Eugenix::Render::OpenGL
