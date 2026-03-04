#pragma once

// ThirdParty headers
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "TestUtils.h"

// Sandbox headers
#include "App/SandboxApp.h"
#include "Render/OpenGL/Buffer.h"
#include "Render/OpenGL/Commands.h"
#include "Render/OpenGL/ShaderProgram.h"
#include "Render/OpenGL/VertexArray.h"

#include "Render/Types.h"
#include "Render/Vertex.h"

namespace
{
	std::array<Eugenix::Render::Vertex::PosColor, 3> triangle_vertices = 
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
			createProgram();
			createGeometry();

			return true;
		}

		void onUpdate(float deltaTime) override
		{
			_xPos += deltaTime * (_invertPos ? -1.0f : 1.0f);
			if (_xPos < -1.0f || _xPos > 1.0f)
				_invertPos = !_invertPos;

			triangle_vertices[1].pos.x = _xPos;

			_triangleVbo.Update(Core::MakeData(&triangle_vertices));
		}

		void onRender() override
		{
			Render::OpenGL::Commands::Viewport(0, 0, width(), height());
			Render::OpenGL::Commands::Clear(Render::ClearFlags::Color);

			_shaderProgram.Bind();

			_triangleVao.Bind();
			Render::OpenGL::Commands::DrawVertices(Render::PrimitiveType::Triangles, 3);
		}
		 
		void onCleanup() override
		{
			_shaderProgram.Destroy();

			_triangleVao.Destroy();
			_triangleVbo.Destroy();
		}

		void onKeyHandle(int key, int code, int action, int mode) override
		{
			if (key == GLFW_KEY_TAB && action == GLFW_PRESS)
			{
				_drawDynamic = !_drawDynamic;
			}
		}

	private:
		void createProgram()
		{
			_shaderProgram = Eugenix::MakeProgramFromFiles("Shaders/simple_pos_color.vert", "Shaders/simple_pos_color.frag");
		}

		void createGeometry()
		{
			_triangleVbo.Create();
			_triangleVbo.Storage(Core::MakeData(&triangle_vertices), GL_DYNAMIC_STORAGE_BIT);

			_triangleVao.Create();
			_triangleVao.AttachVertices(_triangleVbo, sizeof(float) * 6);

			for (const auto& attrib : Render::Vertex::PosColor::layout)
			{
				_triangleVao.Attribute(attrib);
			}
		}

		Render::OpenGL::VertexArray _triangleVao{};
		Render::OpenGL::Buffer _triangleVbo{};
		bool _drawDynamic;
		float _xPos;
		bool _invertPos;

		Render::OpenGL::ShaderProgram _shaderProgram{};
	};
}
