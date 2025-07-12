#pragma once

#include <span>

#include <glm/glm.hpp>

#include "Render/OpenGL/Commands.h"
#include "Render/OpenGL/ShaderStage.h"
#include "Render/OpenGL/VertexArray.h"

// TMP bridge beetween tests & sandbox sources
namespace Eugenix
{
	struct SimpleMesh
	{
		SimpleMesh(std::span<const float> vertices, std::span<const uint32_t> indices)
			: indexCount{ static_cast<uint32_t>(indices.size()) }
		{
			_vao.Create();

			_vbo.Create();
			_vbo.Storage(Eugenix::Core::MakeData(vertices));

			_ibo.Create();
			_ibo.Storage(Eugenix::Core::MakeData(indices));

			constexpr Eugenix::Render::Attribute position_attribute{ 0, 3, GL_FLOAT, GL_FALSE,  0 };

			_vao.AttachVertices(_vbo, sizeof(glm::vec3));
			_vao.AttachIndices(_ibo);
			_vao.Attribute(position_attribute);
		}

		void RenderMesh()
		{
			_vao.Bind();
			Eugenix::Render::OpenGL::Commands::DrawIndexed(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT);
		}

		void ClearMesh()
		{
			_vbo.Destroy();
			_ibo.Destroy();
			_vao.Destroy();

			indexCount = 0;
		}

		~SimpleMesh()
		{
			ClearMesh();
		}

		uint32_t indexCount = 0;

		Eugenix::Render::OpenGL::VertexArray _vao{};
		Eugenix::Render::OpenGL::Buffer _vbo{};
		Eugenix::Render::OpenGL::Buffer _ibo{};
	};

	Eugenix::Render::OpenGL::ShaderStage CreateStage(const char* source, Eugenix::Render::ShaderStageType type)
	{
		Eugenix::Render::OpenGL::ShaderStage stage{ type };
		stage.Create();
		stage.CompileFromSource(source);
		return stage;
	}
}
