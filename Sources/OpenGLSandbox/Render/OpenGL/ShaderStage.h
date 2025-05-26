#pragma once

#include <vector>

#include <glad/glad.h>

#include "Engine/Core/Log.h"

#include "Object.h"
#include "OpenGLTypes.h"

namespace Eugenix
{
	namespace Render::OpenGL
	{
		class ShaderStage final : public Object
		{
		public:
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

			void CompileFromSource(const char* source)
			{
				glShaderSource(_handle, 1, &source, nullptr);
				glCompileShader(_handle);
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

					EUGENIX_ERROR("Shader compile-time error");
				}
			}

			ShaderStageType _stageType{};
		};
	} // namespace Render::OpenGL
} // namespace Eugenix
