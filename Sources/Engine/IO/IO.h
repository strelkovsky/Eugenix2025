#pragma once

//#include <expected>

#include <cassert>
#include <filesystem>
#include <fstream>
#include <string_view>
#include <vector>

namespace Eugenix::IO
{
    struct File
    {
        File() = delete;

        static std::vector<char> ReadText(const std::filesystem::path& path, int32_t mode = 0)
        {
            assert(std::filesystem::is_regular_file(path));

            std::ifstream stream(path, std::ios::ate | mode);
            assert(stream.is_open());

            const auto end = stream.tellg();
            assert(end > 0);

            const auto size = static_cast<std::size_t>(end);
            std::vector<char> content(size + 1);
            
            stream.seekg(0, std::ios::beg);
            if (size > 0)
            {
                stream.read(content.data(), size);
            }
            content[size] = '\0';

            return content;
        }

        static std::vector<char> ReadBinary(const std::filesystem::path& path, int32_t mode = 0)
        {
            assert(std::filesystem::is_regular_file(path));

            std::ifstream stream(path, std::ios::ate | mode);
            assert(stream.is_open());

            const auto end = stream.tellg();
            assert(end > 0);

            const auto size = static_cast<std::size_t>(end);
            std::vector<char> content(size);

            stream.seekg(0, std::ios::beg);
            if (size > 0)
            {
                stream.read(content.data(), size);
            }

            return content;
        }
    };
} // namespace Eugenix::IO
