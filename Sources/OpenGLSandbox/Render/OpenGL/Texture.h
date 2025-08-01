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

			Parameter(TextureParam::WrapS, TextureWrapping::Repeat);
			Parameter(TextureParam::WrapT, TextureWrapping::Repeat);
			Parameter(TextureParam::MinFilter, TextureFilter::Linear);
			Parameter(TextureParam::MagFilter, TextureFilter::Linear);

			glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, imageData.width, imageData.height, 0, dataFormat, to_opengl_type(DataType::UByte), imageData.pixels.get());
			glGenerateMipmap(GL_TEXTURE_2D);

			glBindTexture(GL_TEXTURE_2D, 0);
		}

		// TODO : Sampler
		void Parameter(TextureParam param, TextureWrapping wrapping)
		{
			glTexParameteri(GL_TEXTURE_2D, to_opengl_type(param), to_opengl_type(wrapping));
		}

		// TODO : Sampler
		void Parameter(TextureParam param, TextureFilter filter)
		{
			glTexParameteri(GL_TEXTURE_2D, to_opengl_type(param), to_opengl_type(filter));
		}

		void Bind(uint32_t unit = 0)
		{
			glActiveTexture(GL_TEXTURE0 + unit);
			glBindTexture(GL_TEXTURE_2D, _handle);
		}
	};
}
