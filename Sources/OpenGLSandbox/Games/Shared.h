#pragma once

#include <cstdint>
#include <numbers>

#include "Assets/AssimpModelLoader.h"

namespace core::primitive
{
    struct line
    {
        uint32_t a;
        uint32_t b;

        static constexpr uint32_t elements{ 2 };
    };

    struct triangle
    {
        uint32_t a;
        uint32_t b;
        uint32_t c;

        static constexpr uint32_t elements{ 3 };
    };
}

namespace core::data
{
    template <typename vertex, typename primitive>
    struct geometry
    {
        std::vector<vertex>    vertices;
        std::vector<primitive> elements;
    };
}