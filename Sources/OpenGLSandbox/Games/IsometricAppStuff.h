#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/noise.hpp>
#include <glm/gtc/random.hpp>

namespace Game
{
	enum MapTileType
	{
		Ground,
		Water
	};

	struct MapTile
	{
		glm::vec2 position;
		MapTileType type;
	};

	class Map
	{
	public:
		auto Generate(float tile_half_width, float tile_half_height) -> void
		{
			srand(time(0));

			const auto noise_seed_factor = glm::linearRand(1.0f, 10.0f);

			constexpr auto tile_rows = 6;
			constexpr auto tile_cols = 6;

			for (auto row = 0; row < tile_rows; row++)
			{
				const auto offset_x = row * tile_half_width;
				const auto offset_y = row * tile_half_height;

				for (auto column = 0; column < tile_cols; column++)
				{
					auto& map_tile = tiles[row][column];

					constexpr auto noise_x_factor = 0.2f;
					constexpr auto noise_y_factor = 0.2f;

					const auto noise = glm::simplex(glm::vec3(row * noise_x_factor, column * noise_y_factor, noise_seed_factor));

					if (noise <= -0.5f)
					{
						map_tile.type = Water;
					}
					else
					{
						map_tile.type = Ground;
					}

					const auto x = offset_x + column * tile_half_width;
					const auto y = offset_y - column * tile_half_height;

					map_tile.position = { x, y };
				}
			}
		}

		auto GenerateCorners() -> void
		{

		}

		MapTile tiles[8][8];
	};
}
