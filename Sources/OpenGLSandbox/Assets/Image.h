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
} // namespace Eugenix::Assets
