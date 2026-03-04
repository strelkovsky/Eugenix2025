#pragma once

#include "TestUtils.h"

#include "Engine/Math/Transform.h"

// Sandbox headers
#include "App/SandboxApp.h"
#include "Assets/ImageLoader.h"
#include "Render/OpenGL/Buffer.h"
#include "Render/OpenGL/Commands.h"
#include "Render/OpenGL/ShaderProgram.h"
#include "Render/OpenGL/Texture2D.h"
#include "Render/OpenGL/Sampler.h"
#include "Render/OpenGL/VertexArray.h"
#include "Render/SharedData.h"

namespace Eugenix
{
	class TextureApp final : public SandboxApp
	{
	protected:
		bool onInit() override
		{
			createGeometry();
			createShaders();
			createTextures();
			createUBOs();

			Render::OpenGL::Pipeline::Enable(Render::PipelineFeature::DepthTest);
			Render::OpenGL::Commands::Clear(0.2f, 0.0f, 0.2f);

			_camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f, 2.0f, 0.1f);

			_cameraData.proj = glm::perspective(45.0f, (GLfloat)width() / (GLfloat)height(), 0.1f, 100.0f);

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

			_cameraData.view = _camera.CalculateViewMatrix();
		}

		void onRender() override
		{
			Render::OpenGL::Commands::Viewport(0, 0, width(), height());
			Render::OpenGL::Commands::Clear(Render::ClearFlags::Color | Render::ClearFlags::Depth);

			_cameraUbo.Update(Core::MakeData(&_cameraData));

			_program.Bind();
			{
				_sampler.Bind(0);

				_transform.Reset();
				_transform.Translate({ -0.6f, 0.0f, -3.0f });
				_transform.Scale({ 0.5f, 0.5f, 0.75f });
				_transformUbo.Update(Core::MakeData(&_transform.Matrix()));

				_brickTexture.Bind();
				_meshes[0].Bind();
				_meshes[0].Draw();

				_transform.Reset();
				_transform.Translate({ 0.6f, 0.0f, -3.0f });
				_transform.Scale({ 0.5f, 0.5f, 0.75f });
				_transformUbo.Update(Core::MakeData(&_transform.Matrix()));
				
				_sampler.Bind(0);
				_dirtTexture.Bind();
				_meshes[1].Bind();
				_meshes[1].Draw();
			}
		}

	private:
		void createGeometry()
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

		void createShaders()
		{
			_program = MakeProgramFromFiles("Shaders/texture_shader_ubo.vert", "Shaders/texture_shader_ubo.frag");
		}

		void createTextures()
		{
			const auto brickData = _imageLoader.Load("Textures/brick.png");
			_brickTexture.Create();
			_brickTexture.Upload(brickData);

			const auto dirtData = _imageLoader.Load("Textures/dirt.png");
			_dirtTexture.Create();
			_dirtTexture.Upload(dirtData);
		}

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

		std::vector<Render::Mesh> _meshes;

		Render::OpenGL::ShaderProgram _program{};

		Camera _camera{};

		Assets::ImageLoader _imageLoader{};

		Render::OpenGL::Texture2D _brickTexture;
		Render::OpenGL::Texture2D _dirtTexture;
		Render::OpenGL::Sampler _sampler;

		Render::OpenGL::Buffer _transformUbo{};
		Render::OpenGL::Buffer _cameraUbo{};

		Math::Transform _transform{};
		Render::Data::Camera _cameraData{};
	};
}
