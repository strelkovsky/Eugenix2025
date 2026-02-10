#pragma once

#include <span>

#include "SandboxCompileConfig.h"

#include "Object.h"
#include "Render/Attribute.h"
#include "Buffer.h"

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

			void AttachVertices(const Buffer& buffer, GLint stride)
			{
				//assert(bufferHandle > 0);
				glVertexArrayVertexBuffer(_handle, 0, buffer.NativeHandle(), 0, stride);
			}

			void AttachIndices(const Buffer& buffer)
			{
				//assert(bufferHandle > 0);
				glVertexArrayElementBuffer(_handle, buffer.NativeHandle());
			}

			void Attribute(const Attribute& attribute)
			{
				glVertexArrayAttribFormat(_handle, attribute.index, attribute.size, to_opengl_type(attribute.type), attribute.normalized ? GL_TRUE : GL_FALSE, attribute.offset);
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
