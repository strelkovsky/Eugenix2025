#pragma once

#include <cassert>
#include <fstream>
#include <string>
#include <vector>

namespace Eugenix
{
	namespace IO
	{
        std::vector<char> FileContent(const std::string& path, int32_t mode = 0)
        {
            std::ifstream stream(path, std::ios::ate | mode);
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
