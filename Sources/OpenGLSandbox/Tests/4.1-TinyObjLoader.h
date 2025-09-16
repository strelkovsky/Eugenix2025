#pragma once

#include <filesystem>
#include <unordered_map>
#include <vector>

#include <glm/gtc/matrix_transform.hpp>

#include <tiny_obj_loader.h>

#include "Core/Log.h"

#include "App/SandboxApp.h"
#include "Assets/Image.h"
#include "Assets/ObjModelLoader.h"
#include "Render/OpenGL/Pipeline.h"
#include "Render/OpenGL/Texture2D.h"

#include "Render/Material.h"
#include "Render/Mesh.h"
#include "Render/Model.h"
#include "Render/Vertex.h"

namespace Eugenix
{
	class TinyObjLoaderApp final : public SandboxApp
	{
	protected:
		bool onInit() override
		{
			glEnable(GL_DEPTH_TEST);

			_camera = Camera(glm::vec3(0.0f, 1.0f, 1.5f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, -20.0f, 2.0f, 0.1f);

			_pipeline = MakePipelineFromFiles("Shaders/simple_model_shader.vert", "Shaders/simple_model_shader.frag");

			auto data = _imageLoader.Load("Textures/stone03b.jpg");
			
			_texture.Create();
			_texture.Storage(data);
			_texture.Update(data);

			data = _imageLoader.Load("Textures/uvtestgrid.png");

			_texture2.Create();
			_texture2.Storage(data);
			_texture2.Update(data);

			_sampler.Create();
			_sampler.Parameter(Render::TextureParam::WrapS, Render::TextureWrapping::Repeat);
			_sampler.Parameter(Render::TextureParam::WrapT, Render::TextureWrapping::Repeat);
			_sampler.Parameter(Render::TextureParam::MinFilter, Render::TextureFilter::Linear);
			_sampler.Parameter(Render::TextureParam::MagFilter, Render::TextureFilter::Linear);

			_model = _modelLoader.Load("Models/rock.obj", "Models/");
			_model2 = _modelLoader.Load("Models/backpack/backpack.obj", "Models/backpack/");

			_plane = _modelLoader.Load("Models/plane.obj");

			const std::vector<Render::Vertex::PosNormalUV> vertices =
			{ {
				{ {-1.0f,  1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f} },
				{ { 1.0f,  1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f} },
				{ { 1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f} },
				{ {-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f} },
			} };

			const std::vector<uint32_t> indices = { 0, 1, 2, 2, 3, 0 };

			auto genMesh = Render::Mesh{};
			genMesh.Build(std::span{ vertices }, std::span{indices});

			_customModel.AddPart(genMesh);

			glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
			//glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

			return true;
		}

		void onCleanup() override
		{
			_pipeline.Destroy();

			_texture.Destroy();
			_texture2.Destroy();

			_model.Destroy();
			_model2.Destroy();
			_customModel.Destroy();
			_plane.Destroy();
		}

		void onUpdate(float deltaTime) override
		{
			_camera.keyControl(getKeys(), deltaTime);
			_camera.mouseControl(getMouseButtons(), getXChange(), getYChange());
		}

		void onRender() override
		{
			Render::OpenGL::Commands::Viewport(0, 0, width(), height());
			Render::OpenGL::Commands::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glm::mat4 projection = glm::perspective(45.0f, (GLfloat)width() / (GLfloat)height(), 0.1f, 100.0f);
			_pipeline.SetUniform("view", _camera.CalculateViewMatrix());
			_pipeline.SetUniform("projection", projection);

			_pipeline.Bind();

			// TODO : придумать, где идет связь с семплером
			_sampler.Bind(0);

			glm::mat4 model = glm::mat4(1.0f);

			{
				model = glm::scale(model, { 0.1f, 0.1f, 0.1f });
				model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.0f));
				_pipeline.SetUniform("model", model);
				_model.Render();
			}

			{
				model = glm::mat4(1.0f);
				model = glm::translate(model, { -0.5f, 0.1, 0.0f });
				model = glm::scale(model, { 0.1f, 0.1f, 0.1f });
				_pipeline.SetUniform("model", model);
				_model2.Render();
			}

			{
				model = glm::mat4(1.0f);
				model = glm::translate(model, { 0.5f, 0.2f, 0.4f });
				model = glm::scale(model, { 0.1f, 0.1f, 0.1f });
				_pipeline.SetUniform("model", model);

				_sampler.Bind(0);
				_texture.Bind(0);

				_customModel.Render();
			}

			{
				model = glm::mat4(1.0f);
				model = glm::scale(model, { 5.0f, 1.0f, 5.0f });
				_pipeline.SetUniform("model", model);

				_sampler.Bind(0);
				_texture2.Bind(0);

				_plane.Render();
			}
		}

	private:
		Assets::ImageLoader _imageLoader{};
		Assets::ObjModelLoader _modelLoader{};

		Render::OpenGL::Pipeline _pipeline;
		Render::OpenGL::Texture2D _texture;
		Render::OpenGL::Texture2D _texture2;
		Render::OpenGL::Sampler _sampler;

		Render::Model _model;
		Render::Model _model2;
		Render::Model _customModel;
		Render::Model _plane;

		Camera _camera{};
	};
} // namespace Eugenix