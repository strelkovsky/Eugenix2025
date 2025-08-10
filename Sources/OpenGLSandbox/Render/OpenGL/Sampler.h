#pragma once

#include <glad/glad.h>

#include "Object.h"
#include "OpenGLTypes.h"
#include "Render/Types.h"

namespace Eugenix::Render::OpenGL
{
	class Sampler final : public Object
	{
	public:
		void Create() override
		{
			glCreateSamplers(1, &_handle);
		}

		void Destroy() override
		{
			glDeleteSamplers(1, & _handle);
		}

		// TODO : Sampler
		void Parameter(TextureParam param, TextureWrapping wrapping)
		{
			glSamplerParameteri(_handle, to_opengl_type(param), to_opengl_type(wrapping));
		}

		// TODO : Sampler
		void Parameter(TextureParam param, TextureFilter filter)
		{
			glSamplerParameteri(_handle, to_opengl_type(param), to_opengl_type(filter));
		}

		void Bind(uint32_t location)
		{
			glBindSampler(location, _handle);
		}
	};
} // namespace namespace Eugenix::Render::OpenGL
