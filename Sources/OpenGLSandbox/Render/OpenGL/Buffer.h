#pragma once

#include <glad/glad.h>

#include "Object.h"
#include "OpenGLTypes.h"

#include "Core/Data.h"

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
			glNamedBufferStorage(_handle, data.size, data.ptr, flag);
		}
	};
} // namespace Eugenix::Render::OpenGL
