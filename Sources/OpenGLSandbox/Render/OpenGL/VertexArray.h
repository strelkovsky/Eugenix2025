#pragma once

#include <cassert>

#include <glad/glad.h>

#include "Object.h"
#include "Render/Attribute.h"

namespace Eugenix
{
	namespace Render::OpenGL
	{
		class VertexArray final : public Object
		{
		public:
			void Create() override
			{
				glCreateVertexArrays(1, &_handle);
			}

			void Destroy() override
			{
				glDeleteVertexArrays(1, &_handle);
			}

			void AttachVertices(GLuint bufferHandle, GLint stride)
			{
				//assert(bufferHandle > 0);
				glVertexArrayVertexBuffer(_handle, 0, bufferHandle, 0, stride);
			}

			void AttachIndices(GLuint bufferHandle)
			{
				//assert(bufferHandle > 0);
				glVertexArrayElementBuffer(_handle, bufferHandle);
			}

			void Attribute(const Attribute& attribute)
			{
				glVertexArrayAttribFormat(_handle, attribute.index, attribute.size, attribute.type, attribute.normalized ? GL_TRUE : GL_FALSE, attribute.offset);
				glVertexArrayAttribBinding(_handle, attribute.index, 0);
				glEnableVertexArrayAttrib(_handle, attribute.index);
			}

			void Bind()
			{
				glBindVertexArray(_handle);
			}
		};
	} // namespace Render::OpenGL
} // namespace Eugenix
