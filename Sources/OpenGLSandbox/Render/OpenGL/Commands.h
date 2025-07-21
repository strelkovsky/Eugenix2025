#pragma once

#include <glad/glad.h>

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

	void DrawVertices(uint32_t primitiveType, uint32_t verticesCount, uint32_t first = 0)
	{
		glDrawArrays(primitiveType, first, verticesCount);
	}

	void DrawIndexed(uint32_t primitiveType, uint32_t indicesCount, uint32_t indexType)
	{
		glDrawElements(primitiveType, indicesCount, indexType, nullptr);
	}
} // Eugenix::Render::Opengl
