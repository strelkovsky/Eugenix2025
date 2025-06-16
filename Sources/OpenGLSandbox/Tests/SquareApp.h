#pragma once

// STD headers
#include <iostream>

// ThirdParty headers
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Engine headers
#include "Engine/IO/IO.h"

// Sandbox headers
#include "App/SandboxApp.h"
#include "Render/OpenGL/Buffer.h"
#include "Render/OpenGL/Pipeline.h"
#include "Render/OpenGL/ShaderStage.h"
#include "Render/OpenGL/VertexArray.h"
#include "Render/Types.h"

class SquareApp final : public Eugenix::SandboxApp
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

		_squareVao.Create();

		_squareVbo.Create();
		_squareVbo.Storage(Eugenix::Render::MakeData(square_vertices));

		_squareEbo.Create();
		_squareEbo.Storage(Eugenix::Render::MakeData(square_elements));

		_squareVao.AttachVertices(_squareVbo.Handle(), sizeof(float) * 3);
		_squareVao.AttachIndices(_squareEbo.Handle());

		constexpr Eugenix::Render::Attribute position_attribute{ 0, 3, GL_FLOAT, GL_FALSE,  0 };
		_squareVao.Attribute(position_attribute);

		const auto vsSourceData = Eugenix::IO::FileContent("Shaders/simple.vert");
		const char* vsSource = vsSourceData.data();

		Eugenix::Render::OpenGL::ShaderStage vertexStage{ Eugenix::Render::ShaderStageType::Vertex };
		vertexStage.Create();
		vertexStage.CompileFromSource(vsSource);

		const auto fsSourceData = Eugenix::IO::FileContent("Shaders/simple.frag");
		const char* fsSource = fsSourceData.data();

		Eugenix::Render::OpenGL::ShaderStage fragmentStage{ Eugenix::Render::ShaderStageType::Fragment };
		fragmentStage.Create();
		fragmentStage.CompileFromSource(fsSource);

		_squarePipeline.Create();
		_squarePipeline
			.Attach(vertexStage)
			.Attach(fragmentStage)
			.Build();

		vertexStage.Destroy();
		fragmentStage.Destroy();

		return true;
	}

	void OnRender() override
	{
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		_squarePipeline.Bind();

		_squareVao.Bind();
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
	}

	void OnCleanup() override
	{
		_squarePipeline.Destroy();

		_squareVao.Destroy();
		_squareVbo.Destroy();
		_squareEbo.Destroy();
	}

private:
	Eugenix::Render::OpenGL::VertexArray _squareVao{};
	Eugenix::Render::OpenGL::Buffer _squareVbo{};
	Eugenix::Render::OpenGL::Buffer _squareEbo{};

	Eugenix::Render::OpenGL::Pipeline _squarePipeline{};
};