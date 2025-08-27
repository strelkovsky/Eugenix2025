#pragma once

#include <span>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "TestUtils.h"

// Sandbox headers
#include "App/SandboxApp.h"
#include "Assets/ImageLoader.h"
#include "Render/OpenGL/Buffer.h"
#include "Render/OpenGL/Commands.h"
#include "Render/OpenGL/Pipeline.h"
#include "Render/OpenGL/Texture.h"
#include "Render/OpenGL/Sampler.h"
#include "Render/OpenGL/VertexArray.h"


namespace Eugenix
{
	class TextureApp final : public SandboxApp
	{
	protected:
		bool onInit() override
		{
			CreateGeometry();
			CreatePipelines();

			glEnable(GL_DEPTH_TEST);

			Render::OpenGL::Commands::Clear(0.2f, 0.0f, 0.2f);

			_camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f, 2.0f, 0.1f);

			const auto brickData = _imageLoader.Load("Textures/brick.png");
			
			_brickTexture.Create();
			_brickTexture.Storage(brickData);
			_brickTexture.Update(brickData);

			const auto dirtData = _imageLoader.Load("Textures/brick.png");

			_dirtTexture.Create();
			_dirtTexture.Storage(dirtData);
			_dirtTexture.Update(dirtData);

			_sampler.Create();
			_sampler.Parameter(Render::TextureParam::WrapS, Render::TextureWrapping::Repeat);
			_sampler.Parameter(Render::TextureParam::WrapT, Render::TextureWrapping::Repeat);
			_sampler.Parameter(Render::TextureParam::MinFilter, Render::TextureFilter::Linear);
			_sampler.Parameter(Render::TextureParam::MagFilter, Render::TextureFilter::Linear);

			return true;
		}

		void onUpdate(float deltaTime) override
		{
			_camera.keyControl(getKeys(), deltaTime);
			_camera.mouseControl(getMouseButtons(), getXChange(), getYChange());
		}

		void onRender() override
		{
			Render::OpenGL::Commands::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glm::mat4 projection = glm::perspective(45.0f, (GLfloat)width() / (GLfloat)height(), 0.1f, 100.0f);
			_pipeline.SetUniform("view", _camera.CalculateViewMatrix());
			_pipeline.SetUniform("projection", projection);

			_pipeline.Bind();
			{
				_sampler.Bind(0);

				// Model matrix
				glm::mat4 model = glm::mat4(1.0f);

				model = glm::mat4(1.0f);
				model = glm::translate(model, glm::vec3(-0.6f, 0.0f, -3.0f));
				model = glm::rotate(model, 0.0f, glm::vec3(1.0f, 0.0f, 0.0f));
				model = glm::rotate(model, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
				model = glm::rotate(model, 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
				model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.75f));
				_pipeline.SetUniform("model", model);
				_sampler.Bind(0);
				_brickTexture.Bind();
				_meshes[0].Bind();
				_meshes[0].DrawTriangles();

				model = glm::mat4(1.0f);
				model = glm::translate(model, glm::vec3(0.6f, 0.0f, -3.0f));
				model = glm::rotate(model, 0.0f, glm::vec3(1.0f, 0.0f, 0.0f));
				model = glm::rotate(model, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
				model = glm::rotate(model, 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
				model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.75f));
				_pipeline.SetUniform("model", model);
				_sampler.Bind(0);
				_dirtTexture.Bind();
				_meshes[1].Bind();
				_meshes[1].DrawTriangles();
			}
		}

	private:
		void CreateGeometry()
		{
			const std::array<Eugenix::Render::Vertex::PosUV, 4> verts =
			{ {
				{{-1.0f, -1.0f, 0.0f }, { 0.0f, 0.0f }},
				{{ 0.0f, -1.0f, 1.0f }, { 0.5f, 0.0f }},
				{{ 1.0f, -1.0f, 0.0f }, { 1.0f, 0.0f }},
				{{ 0.0f,  1.0f, 0.0f }, { 0.5f, 1.0f }}
			} };

			const std::array<uint32_t, 12> inds
			{
				0, 3, 1,
				1, 3, 2,
				2, 3, 0,
				0, 1, 2,
			};

			Render::Mesh mesh{};
			mesh.Build(std::span{ verts }, std::span{ inds });

			_meshes.push_back(mesh);
			_meshes.push_back(mesh);
		}

		void CreatePipelines()
		{
			_pipeline.Create();

			const auto vsSourceData = Eugenix::IO::FileContent("Shaders/texture_shader.vert");
			const char* vsSource = vsSourceData.data();

			const auto fsSourceData = Eugenix::IO::FileContent("Shaders/texture_shader.frag");
			const char* fsSource = fsSourceData.data();

			auto vertexStage = Eugenix::CreateStage(vsSource, Eugenix::Render::ShaderStageType::Vertex);
			auto fragmentStage = Eugenix::CreateStage(fsSource, Eugenix::Render::ShaderStageType::Fragment);

			_pipeline.Create();
			_pipeline.AttachStage(vertexStage)
				.AttachStage(fragmentStage)
				.Build();
		}

		std::vector<Render::Mesh> _meshes;

		Render::OpenGL::Pipeline _pipeline{};

		Camera _camera{};

		Assets::ImageLoader _imageLoader{};

		Render::OpenGL::Texture2D _brickTexture;
		Render::OpenGL::Texture2D _dirtTexture;
		Render::OpenGL::Sampler _sampler;
	};
}
