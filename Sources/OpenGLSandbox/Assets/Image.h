#pragma once

#include <memory>

#include <stb_image.h>

namespace Eugenix::Assets
{
	struct ImageData
	{
		ImageData() : pixels(nullptr, stbi_image_free) { }

		int32_t width{};
		int32_t height{};
		int32_t channels{};

		std::unique_ptr<uint8_t[], void(*)(void*)> pixels;
	};

    inline auto MakeEmptyImage(int32_t width, int32_t height, int32_t channels) -> ImageData
    {
        assert(width > 0);
        assert(height > 0);
        assert(channels > 0);

        ImageData data;
        data.width = width;
        data.height = height;
        data.channels = channels;

        const size_t size = static_cast<size_t>(width) * height * channels;

        data.pixels = 
        {
            static_cast<uint8_t*>(std::calloc(size, 1)),
            stbi_image_free
        };

        return data;
    }
} // namespace Eugenix::Assets
