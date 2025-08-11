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
#include "Render/Types.h"
#include "Render/Vertex.h"

namespace
{
	const std::vector<Eugenix::Render::Vertex::PosColor> triangle_vertices
	{
		{{-1.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
		{{ 0.0f,  1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
		{{ 1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}}
	};

//	const std::array<Eugenix::Render::Vertex::PosColor, 3> triangle_vertices2 = 
//	{ {
//		{{-1.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
//		{{ 0.0f,  1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
//		{{ 1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}}
//	} };
//
//	struct Mesh
//	{
//	public:
//		template<class TVertex>
//		void build(std::span<const TVertex> verts,
//			std::span<const uint32_t> indices = {})
//		{
//			_vbo.Create();
//			_vbo.Storage(Eugenix::Core::MakeData(verts));
//
//			_vao.Create();
//			_vao.AttachVertices(_vbo, TVertex::stride);
//			for (auto& a : TVertex::layout) _vao.Attribute(a);
//
//			_indexed = !indices.empty();
//			if (_indexed) {
//				_ebo.Create();
//				_ebo.Storage(Eugenix::Core::MakeData(indices));
//				_vao.AttachIndices(_ebo);
//				_count = static_cast<uint32_t>(indices.size());
//			}
//			else {
//				_count = static_cast<uint32_t>(verts.size());
//			}
//		}
//
//		void bind() { _vao.Bind(); }
//		void drawTriangles() const {
//			if (_indexed) {
//				Eugenix::Render::OpenGL::Commands::DrawIndexed(Eugenix::Render::PrimitiveType::Triangles, _count, Eugenix::Render::DataType::UInt);
//			}
//			else {
//				Eugenix::Render::OpenGL::Commands::DrawVertices(Eugenix::Render::PrimitiveType::Triangles, _count);
//			}
//		}
//
//		void destroy() { _vao.Destroy(); _vbo.Destroy(); _ebo.Destroy(); }
//
//	private:
//		Eugenix::Render::OpenGL::VertexArray _vao{};
//		Eugenix::Render::OpenGL::Buffer _vbo{};
//		Eugenix::Render::OpenGL::Buffer _ebo{};
//		uint32_t _count = 0;
//		bool _indexed = false;
//	};
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

			//_mesh.bind();
			//_mesh.drawTriangles();
		}

	private:
		void createPipeline()
		{
			_pipeline = Eugenix::MakePipeline("Shaders/simple_pos_color.vert", "Shaders/simple_pos_color.frag");
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

			//_mesh.build(std::span{ triangle_vertices2 });
		}

		Render::OpenGL::VertexArray _triangleVao{};
		Render::OpenGL::Buffer _triangleVbo{};

		Render::OpenGL::Pipeline _pipeline{};

		Mesh _mesh;
	};
}
