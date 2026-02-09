#pragma once

//#include <expected>

#include <cassert>
#include <filesystem>
#include <fstream>
#include <string_view>
#include <vector>

// TODO : move to .cpp
namespace
{
    inline std::vector<char> ReadFile(const std::filesystem::path& path,
        std::ios::openmode mode,
        bool addNullTerminator)
    {
        assert(std::filesystem::is_regular_file(path));

        std::ifstream stream(path, std::ios::in | std::ios::ate | mode);
        assert(stream.is_open());

        const std::streamsize size = stream.tellg();
        assert(size >= 0);

        const std::size_t extra = addNullTerminator ? 1u : 0u;
        std::vector<char> data(static_cast<std::size_t>(size) + extra);

        stream.seekg(0, std::ios::beg);

        if (size > 0)
        {
            stream.read(data.data(), size);
        }

        if (addNullTerminator)
            data[static_cast<std::size_t>(size)] = '\0';

        return data;
    }
}

namespace Eugenix::IO
{
    struct File
    {
        File() = delete;

        static std::vector<char> ReadText(const std::filesystem::path& path, std::ios::openmode mode = {})
        {
            return ReadFile(path, /*mode=*/{}, /*addNullTerminator=*/true);
        }

        static std::vector<char> ReadBinary(const std::filesystem::path& path, std::ios::openmode mode = {})
        {
            return ReadFile(path, std::ios::binary, /*addNullTerminator=*/false);
        }
    };
} // namespace Eugenix::IO
