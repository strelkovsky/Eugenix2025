#pragma once

#include <glad/glad.h>

#include "Object.h"
#include "OpenGLTypes.h"

#include "Core/Data.h"

// TODO : remove
#include "../../Tests/TestUtils.h"

namespace Eugenix::Render::OpenGL
{
	class Buffer final : public Object
	{
	public:
		void Create() override
		{
			glCreateBuffers(1, &_handle);
		}

		void Destroy() override
		{
			glDeleteBuffers(1, &_handle);
		}

		void Storage(const Core::Data& data, uint32_t flag = 0)
		{
			assert(data.size > 0 && "Buffer::Storage called with zero size");
			glNamedBufferStorage(_handle, data.size, data.ptr, flag);
		}

		void Update(const Core::Data& data, uint32_t offset = 0)
		{
			assert(data.size > 0 && "Buffer::Update called with zero size");
			// TODO : assert: offset + data.size <= allocatedSize.
			glNamedBufferSubData(_handle, offset, data.size, data.ptr);
		}

		void Bind(BufferTarget target, BufferBinding binding)
		{
			glBindBufferBase(to_opengl_type(target), (GLuint)binding, _handle);
		}
	};
} // namespace Eugenix::Render::OpenGL
