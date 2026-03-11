#pragma once

#include "OpenGLTypes.h"

namespace Eugenix::Render::OpenGL::Pipeline
{
	inline void Enable(PipelineFeature feature)
	{
		glEnable(to_opengl_type(feature));
	}

	inline void Disable(PipelineFeature feature)
	{
		glDisable(to_opengl_type(feature));
	}

	// TODO : use enum
	inline void Blend(uint32_t src, uint32_t dst)
	{
		glBlendFunc(src, dst);
	}

	inline void EnableSolidMode()
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	inline void EnableLinesMode()
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
} // Eugenix::Render::OpenGL::Pipeline