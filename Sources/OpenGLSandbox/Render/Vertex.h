#pragma once

#include <array>
#include <cstddef>

#include <glm/glm.hpp>

#include "Render/Attribute.h"

namespace Eugenix::Render::Vertex
{
	struct Pos
	{
		glm::vec3 pos;

		static constexpr uint32_t stride = sizeof(glm::vec3);

		static constexpr std::array<Render::Attribute, 1> layout =
		{
			Render::Attribute{ /*location*/0, /*size*/3, Render::DataType::Float, false, /*offset*/0 },
		};
	};

	struct PosUV
	{
		glm::vec3 pos;
		glm::vec2 uv;

		static constexpr uint32_t stride = sizeof(glm::vec3) + sizeof(glm::vec2);

		static constexpr std::array<Render::Attribute, 2> layout =
		{
			Render::Attribute{ /*location*/0, /*size*/3, Render::DataType::Float, false, /*offset*/0 },
			Render::Attribute{ /*location*/1, /*size*/2, Render::DataType::Float, false, /*offset*/sizeof(glm::vec3) },
		};
	};

	struct Sprite
	{
		glm::vec2 pos;
		glm::vec2 uv;

		static constexpr uint32_t stride = sizeof(glm::vec2) + sizeof(glm::vec2);

		static constexpr std::array<Render::Attribute, 2> layout =
		{
			Render::Attribute{ /*location*/0, /*size*/2, Render::DataType::Float, false, /*offset*/0 },
			Render::Attribute{ /*location*/1, /*size*/2, Render::DataType::Float, false, /*offset*/sizeof(glm::vec2) },
		};
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

	struct PosNormalUV
	{
		glm::vec3 pos;
		glm::vec3 normal;
		glm::vec2 uv;

		static constexpr uint32_t stride = sizeof(glm::vec3) + sizeof(glm::vec3) + sizeof(glm::vec2);

		static constexpr std::array<Render::Attribute, 3> layout =
		{
			Render::Attribute{ /*location*/0, /*size*/3, Render::DataType::Float, false, /*offset*/0 },
			Render::Attribute{ /*location*/1, /*size*/3, Render::DataType::Float, false, /*offset*/sizeof(glm::vec3) },
			Render::Attribute{ /*location*/2, /*size*/2, Render::DataType::Float, false, /*offset*/sizeof(glm::vec3) + sizeof(glm::vec3) },
		};
	};
} // namespace Eugenix::Render::Vertex