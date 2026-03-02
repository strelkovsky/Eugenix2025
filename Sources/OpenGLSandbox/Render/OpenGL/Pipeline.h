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
} // Eugenix::Render::OpenGL::Pipeline