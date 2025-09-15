#pragma once

#include <array>
#include <span>
#include <string_view>

#include "Assets/ImageLoader.h"
#include "Render/Mesh.h"
#include "Render/Vertex.h"
#include "Render/OpenGL/TextureCubemap.h"
#include "Render/OpenGL/Sampler.h"
#include "Tests/TestUtils.h"

namespace Eugenix::Scene
{
	class Skybox
	{
	public:
		void Create(const std::array<std::string_view, 6> imagePaths)
		{
			const std::array<Eugenix::Render::Vertex::Pos, 24> vertices
			{ {
				//Front
				{{ -1.0f, -1.0f, 1.0f }},
				{ { 1.0, -1.0, 1.0 } },
				{ { 1.0,  1.0, 1.0 } },
				{ { -1.0, 1.0, 1.0 } },
				//Left
				{ { -1.0, -1.0, 1.0 } },
				{ { -1.0, 1.0, 1.0 } },
				{ { -1.0, 1.0, -1.0 } },
				{ { -1.0, -1.0, -1.0 } },
				//Right
				{ { 1.0, -1.0, 1.0 } },
				{ { 1.0, -1.0, -1.0 } },
				{ { 1.0, 1.0, -1.0 } },
				{ { 1.0, 1.0, 1.0 } },
				//Top
				{ { -1.0, 1.0, 1.0 } },
				{ { 1.0, 1.0, 1.0 } },
				{ { 1.0, 1.0, -1.0 } },
				{ { -1.0, 1.0, -1.0 } },
				//Bottom
				{ { -1.0, -1.0, 1.0 } },
				{ { -1.0, -1.0, -1.0 } },
				{ { 1.0, -1.0, -1.0 } },
				{ { 1.0, -1.0, 1.0 } },
				//Back
				{ { -1.0, -1.0, -1.0 } },
				{ { -1.0, 1.0, -1.0 } },
				{ { 1.0, 1.0, -1.0 } },
				{ { 1.0, -1.0, -1.0 } },
			} };

			const std::array<uint32_t, 36> indices =
			{
				0, 2, 1, 3, 2, 0,
				4, 6, 5, 7, 6, 4,
				8, 10, 9, 11, 10, 8,
				12, 14, 13, 15, 14, 12,
				16, 18, 17, 19, 18, 16,
				20, 22, 21, 23, 22, 20
			};

			skyboxMesh.Build(std::span{ vertices }, std::span{ indices });

			pipeline = Eugenix::MakePipelineFromFiles("Shaders/simple_skybox.vert", "Shaders/simple_skybox.frag");
			pipeline.SetUniform("u_cubemap", 0);

			Eugenix::Assets::ImageLoader loader{};

			const std::array<Eugenix::Assets::ImageData, 6> images =
			{
				loader.Load(imagePaths[0]),
				loader.Load(imagePaths[1]),
				loader.Load(imagePaths[2]),
				loader.Load(imagePaths[3]),
				loader.Load(imagePaths[4]),
				loader.Load(imagePaths[5])
			};
			;
			cubemap.Create();
			cubemap.Storage(images);
			cubemap.Update(images);

			sampler.Create();
			sampler.Parameter(Eugenix::Render::TextureParam::MinFilter, Eugenix::Render::TextureFilter::Linear);
			sampler.Parameter(Eugenix::Render::TextureParam::MagFilter, Eugenix::Render::TextureFilter::Linear);
			sampler.Parameter(Eugenix::Render::TextureParam::WrapS, Eugenix::Render::TextureWrapping::ClampToEdge);
			sampler.Parameter(Eugenix::Render::TextureParam::WrapT, Eugenix::Render::TextureWrapping::ClampToEdge);
			sampler.Parameter(Eugenix::Render::TextureParam::WrapR, Eugenix::Render::TextureWrapping::ClampToEdge);
		}

		void render(const glm::mat4& viewProjectionSkyboxMatrix)
		{
			cubemap.Bind();
			sampler.Bind(0);

			skyboxMesh.Bind();

			pipeline.Bind();
			pipeline.SetUniform("u_viewProjectionMatrix", viewProjectionSkyboxMatrix);

			Eugenix::Render::OpenGL::Commands::DepthMask(false);

			skyboxMesh.Draw();

			Eugenix::Render::OpenGL::Commands::DepthMask(true);
		}

	private:
		Eugenix::Render::Mesh skyboxMesh;
		Eugenix::Render::OpenGL::Pipeline pipeline;
		Eugenix::Render::OpenGL::TextureCubemap cubemap;
		Eugenix::Render::OpenGL::Sampler sampler;
	};
} // namespace Eugenix::Scene
