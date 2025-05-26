#pragma once

#include <cassert>

#include <glad/glad.h>

#include "Render/Types.h"

namespace Eugenix
{
	namespace Render::OpenGL
	{
		constexpr GLenum to_opengl_type(ShaderStageType usage)
		{
			switch (usage)
			{
			case ShaderStageType::Vertex : return GL_VERTEX_SHADER;
			case ShaderStageType::Fragment : return GL_FRAGMENT_SHADER;
			case ShaderStageType::Compute : return GL_COMPUTE_SHADER;
			}
			assert(false && "Invalid BufferUsage");
			return 0;
		}
	} // namespace Render::OpenGL
} // namespace Eugenix