#pragma once

#include <memory>

#include "Render/OpenGL/Texture2D.h"

namespace Eugenix::Render
{
	struct Material final
	{
		std::shared_ptr<OpenGL::Texture2D> diffuseTex = {};

		void Destroy()
		{
			diffuseTex.reset();
		}
	};
} // namespace Eugenix::Render

