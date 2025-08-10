#pragma once

#include <glad/glad.h>

#include "Object.h"

#include "Assets/Image.h"
#include "Render/Types.h"

namespace Eugenix::Render::OpenGL
{
	class Texture2D final : public Object
	{
	public:
		void Create() override
		{
			glCreateTextures(GL_TEXTURE_2D, 1, &_handle);
		}

		void Destroy() override
		{
			glDeleteTextures(1, &_handle);
		}

		void Storage(const Assets::ImageData& data, uint32_t levels = 1)
		{
			GLenum internalFormat = 0;
			GLenum dataFormat = 0;
			if (data.channels == 4)
			{
				internalFormat = GL_RGBA8;
				dataFormat = GL_RGBA;
			}
			else if (data.channels == 3)
			{
				internalFormat = GL_RGB8;
				dataFormat = GL_RGB;
			}

			glTextureStorage2D(_handle, levels, internalFormat, data.width, data.height);
		}

		void Update(const Assets::ImageData& data, uint32_t level = 0)
		{
			GLenum internalFormat = 0;
			GLenum dataFormat = 0;
			if (data.channels == 4)
			{
				internalFormat = GL_RGBA8;
				dataFormat = GL_RGBA;
			}
			else if (data.channels == 3)
			{
				internalFormat = GL_RGB8;
				dataFormat = GL_RGB;
			}

			glTextureSubImage2D(_handle, level, 0, 0, data.width, data.height, dataFormat, GL_UNSIGNED_BYTE, data.pixels.get());
		}

		void Bind(uint32_t unit = 0)
		{
			glBindTextureUnit(unit, _handle);
		}
	};
}
