#pragma once

#include <array>
#include <cstddef>

#include <glm/glm.hpp>

#include "Render/Attribute.h"

namespace Eugenix::Render::Vertex
{
	struct Pos
	{

	};

	struct PosColor
	{
		glm::vec3 pos;
		glm::vec3 color;

		static constexpr uint32_t stride = sizeof(glm::vec3) + sizeof(glm::vec3);

		static constexpr std::array<Render::Attribute, 2> layout = 
		{
			Render::Attribute{ /*location*/0, /*size*/3, Render::DataType::Float, false, /*offset*/0 },
			Render::Attribute{ /*location*/1, /*size*/3, Render::DataType::Float, false, /*offset*/sizeof(glm::vec3) },
		};
	};
} // namespace Eugenix::Render::Vertex