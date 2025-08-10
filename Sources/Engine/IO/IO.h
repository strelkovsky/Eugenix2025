#pragma once

#include <cassert>
#include <fstream>
#include <string_view>
#include <vector>

namespace Eugenix
{
	namespace IO
	{
        std::vector<char> FileContent(std::string_view path, int32_t mode = 0)
        {
            std::ifstream stream(std::string(path), std::ios::ate | mode);
            assert(stream.is_open());

            const auto size = stream.tellg();
            assert(size > 0);

            std::vector<char> content(size);

            stream.seekg(0, std::ios::beg);
            stream.read(content.data(), size);

            return content;
        }
	}
} // namespace Eugenix
