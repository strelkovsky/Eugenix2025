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

            const auto end = stream.tellg();
            assert(end > 0);
            const size_t size = static_cast<size_t>(end);

            std::vector<char> content(size+1);

            stream.seekg(0, std::ios::beg);
            stream.read(content.data(), size);
            content[size] = '\0';

            return content;
        }
	}
} // namespace Eugenix
