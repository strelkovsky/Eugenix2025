#pragma once

// STD headers
#include <iostream>

// ThirdParty headers
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "TestUtils.h"

// Engine headers
#include "Engine/IO/IO.h"

// Sandbox headers
#include "App/SandboxApp.h"
#include "Render/OpenGL/Buffer.h"
#include "Render/OpenGL/Commands.h"
#include "Render/OpenGL/Pipeline.h"
#include "Render/OpenGL/ShaderStage.h"
#include "Render/OpenGL/VertexArray.h"
#include "Render/Types.h"

namespace Eugenix
{
	class SquareApp final : public SandboxApp
	{
	protected:
		bool OnInit() override
		{
			const std::vector<float> square_vertices
			{
				-0.5f, -0.5f, 0.0f,
				 0.5f, -0.5f, 0.0f,
				-0.5f,  0.5f, 0.0f,
				 0.5f,  0.5f, 0.0f
			};

			const std::vector<uint32_t> square_elements
			{
				0, 1, 2,
				1, 2, 3
			};

			constexpr Render::Attribute position_attribute{ 0, 3, Render::DataType::Float, GL_FALSE,  0 };

			_squareVbo.Create();
			_squareVbo.Storage(Core::MakeData(std::span{ square_vertices }));

			_squareEbo.Create();
			_squareEbo.Storage(Core::MakeData(std::span{ square_elements }));

			_squareVao.Create();
			_squareVao.AttachVertices(_squareVbo, sizeof(float) * 3);
			_squareVao.AttachIndices(_squareEbo);
			_squareVao.Attribute(position_attribute);

			const auto vsSourceData = Eugenix::IO::FileContent("Shaders/simple.vert");
			const char* vsSource = vsSourceData.data();

			auto vertexStage = CreateStage(vsSource, Eugenix::Render::ShaderStageType::Vertex);

			const auto fsSourceData = Eugenix::IO::FileContent("Shaders/simple.frag");
			const char* fsSource = fsSourceData.data();

			auto fragmentStage = CreateStage(fsSource, Eugenix::Render::ShaderStageType::Fragment);

			_squarePipeline.Create();
			_squarePipeline
				.AttachStage(vertexStage)
				.AttachStage(fragmentStage)
				.Build();

			vertexStage.Destroy();
			fragmentStage.Destroy();

			Render::OpenGL::Commands::Clear(0.2f, 0.3f, 0.3f);

			return true;
		}

		void OnRender() override
		{
			Render::OpenGL::Commands::Clear(GL_COLOR_BUFFER_BIT);

			_squarePipeline.Bind();

			_squareVao.Bind();

			Render::OpenGL::Commands::DrawIndexed(Render::PrimitiveType::Triangles, 6, Render::DataType::UInt);
		}

		void OnCleanup() override
		{
			_squarePipeline.Destroy();

			_squareVao.Destroy();
			_squareVbo.Destroy();
			_squareEbo.Destroy();
		}

	private:
		Render::OpenGL::VertexArray _squareVao{};
		Render::OpenGL::Buffer _squareVbo{};
		Render::OpenGL::Buffer _squareEbo{};

		Render::OpenGL::Pipeline _squarePipeline{};
	};
}