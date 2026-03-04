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
#include "Render/OpenGL/ShaderProgram.h"
#include "Render/OpenGL/Texture2D.h"
#include "Render/OpenGL/Commands.h"

#include "Render/Material.h"
#include "Render/Mesh.h"
#include "Render/Model.h"
#include "Render/Vertex.h"
#include "Render/SharedData.h"

#include "Engine/Math/Transform.h"

namespace Eugenix
{
	class TinyObjLoaderApp final : public SandboxApp
	{
	protected:
		bool onInit() override
		{
			Render::OpenGL::Pipeline::Enable(Render::PipelineFeature::DepthTest);

			_camera = Camera(glm::vec3(0.0f, 1.0f, 1.5f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, -20.0f, 2.0f, 0.1f);

			_program = MakeProgramFromFiles("Shaders/simple_model_shader.vert", "Shaders/simple_model_shader.frag");

			auto data = _imageLoader.Load("Textures/stone03b.jpg");
			_texture.Create();
			_texture.Upload(data);

			data = _imageLoader.Load("Textures/uvtestgrid.png");
			_texture2.Create();
			_texture2.Upload(data);

			_sampler.Create();
			_sampler.Parameter(Render::TextureParam::WrapS, Render::TextureWrapping::Repeat);
			_sampler.Parameter(Render::TextureParam::WrapT, Render::TextureWrapping::Repeat);
			_sampler.Parameter(Render::TextureParam::MinFilter, Render::TextureFilter::Linear);
			_sampler.Parameter(Render::TextureParam::MagFilter, Render::TextureFilter::Linear);

			createUBOs();

			_model = _modelLoader.Load("Models/nanosuit/nanosuit.obj");
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

			Render::OpenGL::Commands::Clear(0.5f, 0.5f, 0.5f);

			_cameraData.proj = glm::perspective(45.0f, (float)width() / (float)height(), 0.1f, 100.0f);

			return true;
		}

		void onCleanup() override
		{
			_program.Destroy();

			_texture.Destroy();
			_texture2.Destroy();

			_model.Destroy();
			_customModel.Destroy();
			_plane.Destroy();
		}

		void onUpdate(float deltaTime) override
		{
			_camera.keyControl(getKeys(), deltaTime);
			_camera.mouseControl(getMouseButtons(), getXChange(), getYChange());

			_cameraData.view = _camera.CalculateViewMatrix();
		}

		void onRender() override
		{
			Render::OpenGL::Commands::Viewport(0, 0, width(), height());
			Render::OpenGL::Commands::Clear(Render::ClearFlags::Color | Render::ClearFlags::Depth);

			//glm::mat4 projection = glm::perspective(45.0f, (GLfloat)width() / (GLfloat)height(), 0.1f, 100.0f);
			//_program.SetUniform("view", _cameraData.view);
			//_program.SetUniform("projection", _cameraData.proj);

			_cameraUbo.Update(Core::MakeData(&_cameraData));

			_program.Bind();

			// TODO : придумать, где идет связь с семплером
			_sampler.Bind(0);

			{
				_transform.Reset();
				_transform.Scale({ 0.1f, 0.1f, 0.1f });
				_transformUbo.Update(Core::MakeData(&_transform.Matrix()));
				_model.Render();
			}

			{
				_transform.Reset();
				_transform.Translate({ 0.5f, 0.2f, 0.4f });
				_transform.Scale({ 0.1f, 0.1f, 0.1f });
				_transformUbo.Update(Core::MakeData(&_transform.Matrix()));

				_sampler.Bind(0);
				_texture.Bind(0);

				_customModel.Render();
			}

			{
				_transform.Reset();
				_transform.Scale({ 5.0f, 1.0f, 5.0f });
				_transformUbo.Update(Core::MakeData(&_transform.Matrix()));

				_sampler.Bind(0);
				_texture2.Bind(0);

				_plane.Render();
			}
		}

	private:
		void createUBOs()
		{
			_transformUbo.Create();
			// TODO : see GP4 (create by size, not by data)
			_transformUbo.Storage(Core::MakeData(&_transform), GL_DYNAMIC_STORAGE_BIT);
			_transformUbo.Bind(Render::BufferTarget::UBO, Render::BufferBinding::Transform);

			_cameraUbo.Create();
			_cameraUbo.Storage(Core::MakeData(&_cameraData), GL_DYNAMIC_STORAGE_BIT);
			_cameraUbo.Bind(Render::BufferTarget::UBO, Render::BufferBinding::Camera);
		}

		Assets::ImageLoader _imageLoader{};
		Assets::ObjModelLoader _modelLoader{};

		Render::OpenGL::ShaderProgram _program;
		Render::OpenGL::Texture2D _texture;
		Render::OpenGL::Texture2D _texture2;
		Render::OpenGL::Sampler _sampler;

		Render::Model _model;
		Render::Model _customModel;
		Render::Model _plane;

		Camera _camera{};

		Math::Transform _transform{};
		Render::Data::Camera _cameraData{};

		Render::OpenGL::Buffer _transformUbo{};
		Render::OpenGL::Buffer _cameraUbo{};
	};
} // namespace Eugenix