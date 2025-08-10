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

namespace
{
	struct testVertex
	{
		glm::vec3 pos;
		glm::vec3 color;
	};

	const std::vector<float> triangle_vertices
	{
		-1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		 0.0f,  1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f
	};
}

namespace Eugenix
{
	class TriangleApp final : public SandboxApp
	{
	protected:
		bool onInit() override
		{
			createPipeline();
			createGeometry();

			return true;
		}

		void onRender() override
		{
			Render::OpenGL::Commands::Clear(GL_COLOR_BUFFER_BIT);

			_pipeline.Bind();

			_triangleVao.Bind();

			Render::OpenGL::Commands::DrawVertices(Render::PrimitiveType::Triangles, 3);
		}

	private:
		void createPipeline()
		{
			_pipeline = Eugenix::MakePipeline("Shaders/simple_pos_color.vert", "Shaders/simple_pos_color.frag");
		}

		void createGeometry()
		{
			constexpr Render::Attribute position_attribute{ 0, 3, Render::DataType::Float, GL_FALSE,  0 };
			constexpr Render::Attribute color_attribute{ 1, 3, Render::DataType::Float, GL_FALSE,  sizeof(glm::vec3) };

			_triangleVbo.Create();
			_triangleVbo.Storage(Core::MakeData(std::span{ triangle_vertices }));

			_triangleVao.Create();
			_triangleVao.AttachVertices(_triangleVbo, sizeof(float) * 6);
			_triangleVao.Attribute(position_attribute);
			_triangleVao.Attribute(color_attribute);
		}

		Render::OpenGL::VertexArray _triangleVao{};
		Render::OpenGL::Buffer _triangleVbo{};

		Render::OpenGL::Pipeline _pipeline{};
	};
}
