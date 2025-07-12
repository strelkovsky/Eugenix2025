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
	} // namespace Render::OpenGL
} // namespace Eugenix
