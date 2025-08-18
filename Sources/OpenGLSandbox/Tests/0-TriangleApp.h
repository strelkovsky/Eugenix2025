#pragma once

// STD headers
#include <array>
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

#include "Render/Mesh.h"
#include "Render/Types.h"
#include "Render/Vertex.h"

namespace
{
	const std::array<Eugenix::Render::Vertex::PosColor, 3> triangle_vertices = 
	{ {
		{{-1.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
		{{ 0.0f,  1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
		{{ 1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}}
	} };
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

			//_triangleVao.Bind();
			//Render::OpenGL::Commands::DrawVertices(Render::PrimitiveType::Triangles, 3);

			_mesh.Bind();
			_mesh.DrawTriangles();
		}
		 
		void onCleanup() override
		{
			_pipeline.Destroy();
			_mesh.Destroy();
		}

	private:
		void createPipeline()
		{
			_pipeline = Eugenix::MakePipelineFromFiles("Shaders/simple_pos_color.vert", "Shaders/simple_pos_color.frag");
		}

		void createGeometry()
		{
			_triangleVbo.Create();
			_triangleVbo.Storage(Core::MakeData(std::span{ triangle_vertices }));

			_triangleVao.Create();
			_triangleVao.AttachVertices(_triangleVbo, sizeof(float) * 6);

			// TODO : push layout in vertex_array
			//for (const auto& attrib : Render::Vertex::PosColor::layout)
			//{
			//	_triangleVao.Attribute(attrib);
			//}

			uint32_t currentOffset = 0;
			for (const auto& attrib : _pipeline.GetAttribs())
			{
				auto a = AttributeFromShader(attrib, currentOffset);
				currentOffset += ComponentsFromGLType(attrib.type) * BytesPerComponent(a.type);
				_triangleVao.Attribute(a);
			}

			_mesh.Build(std::span{ triangle_vertices });
		}

		Render::OpenGL::VertexArray _triangleVao{};
		Render::OpenGL::Buffer _triangleVbo{};

		Render::OpenGL::Pipeline _pipeline{};

		Render::Mesh _mesh;
	};
}
