#pragma once

#include <cassert>

#include <glad/glad.h>

#include "Render/Types.h"

namespace Eugenix
{
	namespace Render::OpenGL
	{
		constexpr GLenum to_opengl_type(DataType type)
		{
			switch (type)
			{
			case DataType::UInt : return GL_UNSIGNED_INT;
			case DataType::Float : return GL_FLOAT;
			}
			assert(false && "Invalid PrimitiveType");
			return 0;
		}

		constexpr GLenum to_opengl_type(PrimitiveType type)
		{
			switch (type)
			{
			case PrimitiveType::Triangles : return GL_TRIANGLES;
			}
			assert(false && "Invalid PrimitiveType");
			return 0;
		}

		constexpr GLenum to_opengl_type(ShaderStageType usage)
		{
			switch (usage)
			{
			case ShaderStageType::Vertex : return GL_VERTEX_SHADER;
			case ShaderStageType::Fragment : return GL_FRAGMENT_SHADER;
			case ShaderStageType::Compute : return GL_COMPUTE_SHADER;
			}
			assert(false && "Invalid ShaderStageType");
			return 0;
		}

		constexpr GLenum to_opengl_type(BufferTarget target)
		{
			switch (target)
			{
			case BufferTarget::Uniform: return GL_UNIFORM_BUFFER;
			}
			assert(false && "Invalid BufferTarget");
			return 0;
		}
	} // namespace Render::OpenGL
} // namespace Eugenix