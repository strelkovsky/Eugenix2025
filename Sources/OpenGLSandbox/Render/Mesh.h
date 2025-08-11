#pragma once

#include <cstddef>
#include <span>

#include "Render/Types.h"

#include "Render/OpenGL/Buffer.h"
#include "Render/OpenGL/Commands.h"
#include "Render/OpenGL/VertexArray.h"

namespace Eugenix::Render
{
	struct Mesh
	{
	public:
		template<class TVertex, std::size_t N>
		void Build(std::span<const TVertex, N> verts,
			std::span<const uint32_t> indices = {})
		{
			_vbo.Create();
			_vbo.Storage(Eugenix::Core::MakeData(verts));

			_vao.Create();
			_vao.AttachVertices(_vbo, TVertex::stride);

			for (auto& a : TVertex::layout)
			{
				_vao.Attribute(a);
			}

			_indexed = !indices.empty();
			if (_indexed) 
			{
				_ebo.Create();
				_ebo.Storage(Eugenix::Core::MakeData(indices));
				_vao.AttachIndices(_ebo);
				_count = static_cast<uint32_t>(indices.size());
			}
			else 
			{
				_count = static_cast<uint32_t>(verts.size());
			}
		}

		void Bind() 
		{ 
			_vao.Bind(); 
		}

		void DrawTriangles() const 
		{
			if (_indexed) 
			{
				Render::OpenGL::Commands::DrawIndexed(Render::PrimitiveType::Triangles, _count, Render::DataType::UInt);
			}
			else 
			{
				Render::OpenGL::Commands::DrawVertices(Render::PrimitiveType::Triangles, _count);
			}
		}

		void Destroy() 
		{ 
			_vao.Destroy();
			_vbo.Destroy();
			_ebo.Destroy(); 
		}

	private:
		Render::OpenGL::VertexArray _vao{};
		Render::OpenGL::Buffer _vbo{};
		Render::OpenGL::Buffer _ebo{};
		uint32_t _count = 0;
		bool _indexed = false;
	};
} // namespace Eugenix::Render