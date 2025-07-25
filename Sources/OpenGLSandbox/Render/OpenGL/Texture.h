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

		void Storage(const Assets::ImageData& imageData)
		{
			GLenum internalFormat = 0;
			GLenum dataFormat = 0;
			if (imageData.channels == 4)
			{
				internalFormat = GL_RGBA8;
				dataFormat = GL_RGBA;
			}
			else if (imageData.channels == 3)
			{
				internalFormat = GL_RGB8;
				dataFormat = GL_RGB;
			}

			glBindTexture(GL_TEXTURE_2D, _handle);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, imageData.width, imageData.height, 0, dataFormat, GL_UNSIGNED_BYTE, imageData.pixels.get());
			glGenerateMipmap(GL_TEXTURE_2D);

			glBindTexture(GL_TEXTURE_2D, 0);
		}

		void Bind(uint32_t unit = 0)
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, _handle);
		}
	};
}
