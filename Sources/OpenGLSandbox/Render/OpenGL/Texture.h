#pragma once

#include <glad/glad.h>

#include "Object.h"

#include "Assets/Image.h"

namespace Eugenix::Render::OpenGL
{
	class Texture final : public Object
	{
	public:
		void Create() override
		{
			//glCreateTextures(GL_TEXTURE_2D, 1, &_handle);
			glGenTextures(1, &_handle);
		}

		void Destroy() override
		{
			glDeleteTextures(1, &_handle);
		}

		void Storage()
		{
			
		}

		void Bind(uint32_t unit = 0)
		{

		}
	};
}
