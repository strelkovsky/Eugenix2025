#pragma once

#include "OGLDevMath.h"

struct Vertex
{
    glm::vec3 pos;
    glm::vec2 uv;

    Vertex() {}

    Vertex(const glm::vec3& pos_, const glm::vec2& uv_)
    {
        pos = pos_;
        uv = uv_;
    }
};
