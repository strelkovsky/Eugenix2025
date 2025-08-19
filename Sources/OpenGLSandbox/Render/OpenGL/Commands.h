#pragma once

#include <glad/glad.h>

#include "OpenGLTypes.h"

namespace Eugenix::Render::OpenGL::Commands
{
	inline void Clear(float r, float g, float b)
	{
		glClearColor(r, g, b, 1.0f);
	}

	inline void Clear(uint32_t mask)
	{
		glClear(mask);
	}

	inline void Viewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		glViewport(x, y, width, height);
	}

	inline void DepthMask(bool enable)
	{
		glDepthMask(enable ? GL_TRUE : GL_FALSE);
	}

	inline void DrawVertices(PrimitiveType primitiveType, uint32_t verticesCount, uint32_t first = 0)
	{
		glDrawArrays(to_opengl_type(primitiveType), first, verticesCount);
	}

	inline void DrawIndexed(PrimitiveType primitiveType, uint32_t indicesCount, DataType indexType)
	{
		glDrawElements(to_opengl_type(primitiveType), indicesCount, to_opengl_type(indexType), nullptr);
	}
} // Eugenix::Render::Opengl
