#pragma once

#include <glad/glad.h>

#include "Object.h"
#include "ShaderStage.h"

namespace Eugenix
{
	namespace Render::OpenGL
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

			Pipeline& Attach(ShaderStage& stage)
			{
				glAttachShader(_handle, stage.Handle());
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

		private:
			void checkLinkStatus()
			{
				GLint success;
				glGetProgramiv(_handle, GL_LINK_STATUS, &success);
				if (!success)
				{
					GLint logLength;
					glGetProgramiv(_handle, GL_INFO_LOG_LENGTH, &logLength);
					std::vector<char> shaderLog(logLength);
					glGetProgramInfoLog(_handle, logLength, nullptr, shaderLog.data());

					EUGENIX_ERROR("Program link error");
				}
			}
		};
	} // namespace Render::OpenGL
} // namespace Eugenix
