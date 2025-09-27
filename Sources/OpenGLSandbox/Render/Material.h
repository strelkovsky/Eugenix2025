#pragma once

#include <memory>

#include "Render/OpenGL/Texture2D.h"

namespace Eugenix::Render
{
	struct Material final
	{
		std::shared_ptr<OpenGL::Texture2D> diffuseTex = {};
		std::shared_ptr<OpenGL::Texture2D> normalsTex = {};
		std::shared_ptr<OpenGL::Texture2D> specularTex = {};

		void Destroy()
		{
			diffuseTex.reset();
			normalsTex.reset();
			specularTex.reset();
		}
	};
} // namespace Eugenix::Render

