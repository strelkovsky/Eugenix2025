#pragma once

#include "SandboxCompileConfig.h"

#include "Object.h"

#include "Assets/Image.h"
#include "Render/Types.h"

namespace
{
	inline std::pair<GLenum, GLenum> ChooseTextureFormat(int channels, bool srgb)
	{
		GLenum internalFormat = GL_RGBA8;
		GLenum dataFormat = GL_RGBA;

		switch (channels)
		{
		case 1:
			internalFormat = GL_R8;
			dataFormat = GL_RED;
			break;

		case 2:
			internalFormat = GL_RG8;
			dataFormat = GL_RG;
			break;

		case 3:
			internalFormat = srgb ? GL_SRGB8 : GL_RGB8;
			//internalFormat = GL_RGB8;
			dataFormat = GL_RGB;
			break;

		case 4:
			internalFormat = srgb ? GL_SRGB8_ALPHA8 : GL_RGBA8;
			//internalFormat = GL_RGBA8;
			dataFormat = GL_RGBA;
			break;

		default:
			internalFormat = srgb ? GL_SRGB8 : GL_RGB8;
			dataFormat = GL_RGB;
			break;
		}

		return { internalFormat, dataFormat };
	}
}

namespace Eugenix::Render::OpenGL
{
	struct TextureDesc
	{
		TextureColorSpace colorSpace = TextureColorSpace::SRGB;
		uint32_t mipLevels = 0;
		bool generateMipmaps = true;
	};

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

		void Upload(const Assets::ImageData& img, const TextureDesc& desc = {})
		{
			// allocate
			Storage(img.width, img.height, img.channels, desc);
			// upload base level
			Update(img, 0, desc.colorSpace);
			// build mips
			if ((desc.mipLevels == 0 || desc.mipLevels > 1) && desc.generateMipmaps)
				GenerateMipmaps();
		}

		void Storage(int width, int height, int channels, const TextureDesc& desc)
		{
			const bool srgb = (desc.colorSpace == TextureColorSpace::SRGB);
			printf("is sRGB - %s\n", srgb ? "yes" : "no");
			auto [internalFormat, _] = ChooseTextureFormat(channels, srgb);

			uint32_t levels = desc.mipLevels ? desc.mipLevels
				: (1u + (uint32_t)std::floor(std::log2(std::max(width, height))));

			glTextureStorage2D(_handle, levels, internalFormat, width, height);
		}

		void Update(const Assets::ImageData& data, uint32_t level, TextureColorSpace colorSpace)
		{
			const bool srgb = (colorSpace == TextureColorSpace::SRGB);
			auto [_, dataFormat] = ChooseTextureFormat(data.channels, srgb);
			glTextureSubImage2D(_handle, level, 0, 0, data.width, data.height, dataFormat, GL_UNSIGNED_BYTE, data.pixels.get());
		}

		void GenerateMipmaps()
		{
			glGenerateTextureMipmap(_handle);
		}

		// TODO : Bind(location::albedo) - вынести в RenderSharedData, как и настройки UBO

		void Bind(uint32_t unit = 0)
		{
			glBindTextureUnit(unit, _handle);
		}
	};
}
