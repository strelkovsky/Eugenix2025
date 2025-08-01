#pragma once

#include <string_view>

#include <stb_image.h>

#include "Image.h"
#include "Core/Log.h"

namespace Eugenix::Assets
{
	class ImageLoader
	{
	public:
		ImageData Load(std::string_view name)
		{
			ImageData data{};

			//stbi_set_flip_vertically_on_load(1);

			uint8_t* raw = stbi_load(name.data(), &data.width, &data.height, &data.channels, 0);
			if (!raw)
			{
				LogError("Failed to load image at path", name);
				return {};
			}

			data.pixels.reset(raw);
			return data;
		}
	};
} // namespace Eugenix::Assets
