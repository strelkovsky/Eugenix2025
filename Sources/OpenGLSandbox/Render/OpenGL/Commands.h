#pragma once

#include <glad/glad.h>

#include "OpenGLTypes.h"

namespace Eugenix::Render::OpenGL::Commands
{
	void Clear(float r, float g, float b)
	{
		glClearColor(r, g, b, 1.0f);
	}

	void Clear(uint32_t mask)
	{
		glClear(mask);
	}

	void DrawVertices(PrimitiveType primitiveType, uint32_t verticesCount, uint32_t first = 0)
	{
		glDrawArrays(to_opengl_type(primitiveType), first, verticesCount);
	}

	void DrawIndexed(PrimitiveType primitiveType, uint32_t indicesCount, DataType indexType)
	{
		glDrawElements(to_opengl_type(primitiveType), indicesCount, to_opengl_type(indexType), nullptr);
	}
} // Eugenix::Render::Opengl
