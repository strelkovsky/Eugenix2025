#pragma once

#include <glm/glm.hpp>

namespace Eugenix::Render::Data
{
	// UBOs
	struct Camera
	{
		glm::mat4 view;
		glm::mat4 proj;
	};
	
} // namespace Eugenix::Render::Data
