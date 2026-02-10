#pragma once

#include "SandboxCompileConfig.h"

#ifdef EUGENIX_OPENGL_GLAD
#	include <glad/glad.h>
#endif // EUGENIX_OPENGL_GLAD

namespace Eugenix::Render::OpenGL
{
	bool Init()
	{
#ifdef EUGENIX_OPENGL_GLAD
		return gladLoadGL();
#endif // EUGENIX_OPENGL_GLAD
	}
} // Eugenix::Render::OpenGL