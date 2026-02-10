#pragma once

#include <array>

#include "SandboxCompileConfig.h"

#include "Assets/Image.h"

#include "Render/OpenGL/Object.h"
#include "Render/OpenGL/OpenGLTypes.h"

namespace Eugenix::Render::OpenGL
{
	class TextureCubemap final : public Object
	{
	public:
		void Create() override
		{
			glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &_handle);
		}

		void Destroy() override
		{
			glDeleteTextures(1, &_handle);
		}

		void Storage(const std::array<Eugenix::Assets::ImageData, 6>& images)
		{
			GLenum internalFormat;
			GLenum type;

			if (images[0].channels == 3)
			{
				internalFormat = GL_RGB8;
				type = GL_RGB;
			}
			else
			{
				internalFormat = GL_RGBA8;
				type = GL_RGBA;
			}

			glTextureStorage2D(_handle, 1, internalFormat, images[0].width, images[0].height);
		}

		void Update(const std::array<Eugenix::Assets::ImageData, 6>& images)
		{
			GLenum internalFormat;
			GLenum type;

			if (images[0].channels == 3)
			{
				internalFormat = GL_RGB8;
				type = GL_RGB;
			}
			else
			{
				internalFormat = GL_RGBA8;
				type = GL_RGBA;
			}

			for (GLint face = 0; face < 6; ++face)
			{
				glTextureSubImage3D(
					_handle,
					/*level*/ 0,
					/*xoff*/ 0, /*yoff*/ 0, /*zoff*/ face,
					/*w*/ images[0].width, /*h*/ images[0].height, /*d*/ 1,
					type,
					Eugenix::Render::OpenGL::to_opengl_type(Eugenix::Render::DataType::UByte),
					images[face].pixels.get()
				);
			}

			glTextureParameteri(_handle, GL_TEXTURE_BASE_LEVEL, 0);
			glTextureParameteri(_handle, GL_TEXTURE_MAX_LEVEL, 0);
		}

		void Bind(uint32_t unit = 0) const
		{
			glBindTextureUnit(unit, _handle);
		}
	};
} // namespace Eugenix::Render::OpenGL