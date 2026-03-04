#pragma once

#include "TestUtils.h"

// Sandbox headers
#include "App/SandboxApp.h"
#include "Render/OpenGL/Buffer.h"
#include "Render/OpenGL/Commands.h"
#include "Render/Mesh.h"
#include "Render/OpenGL/ShaderProgram.h"
#include "Render/OpenGL/VertexArray.h"
#include "Render/OpenGL/OpenGLTypes.h"
#include "Render/OpenGL/TextureCubemap.h"

#include "Scene/Skybox.h"

#include "Engine/Math/Transform.h"

namespace Eugenix
{
	class SkyboxApp final : public SandboxApp
	{
	protected:
		bool onInit() override
		{
			createGeometry();
			createPipelines();
			createSkybox();
			createUBOs();

			Render::OpenGL::Pipeline::Enable(Render::PipelineFeature::DepthTest);
			Render::OpenGL::Commands::Clear(0.2f, 0.0f, 0.2f);

			_camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f, 2.0f, 0.1f);

			_cameraData.proj = glm::perspective(45.0f, (float)width() / (float)height(), 0.1f, 100.0f);

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

			Render::OpenGL::Commands::DepthMask(true);

			_cameraUbo.Update(Core::MakeData(&_cameraData));
			skybox.render(_cameraData.proj * glm::mat4(glm::mat3(_cameraData.view)));

			_program.Bind();

			{
				_transform.Reset();
				_transform.Translate({ -0.6f, 0.0f, -3.0f });
				_transform.Scale({ 0.5f, 0.5f, 0.75f });
				_transformUbo.Update(Core::MakeData(&_transform.Matrix()));
				_meshes[0].Bind();
				_meshes[0].Draw();

				_transform.Reset();
				_transform.Translate({ 0.6f, 0.0f, -3.0f });
				_transform.Scale({ 0.5f, 0.5f, 0.75f });
				_transformUbo.Update(Core::MakeData(&_transform.Matrix()));
				_meshes[1].Bind();
				_meshes[1].Draw();

				_transform.Reset();
				_transform.Translate({ 0.0f, 0.0f, -3.0f });
				_transform.Scale({ 0.5f, 0.5f, 0.75f });
				_transformUbo.Update(Core::MakeData(&_transform.Matrix()));
				_mesh.Bind();
				_mesh.Draw();
			}
		}

		void onResize() override
		{
			_cameraData.proj = glm::perspective(45.0f, (float)width() / (float)height(), 0.1f, 100.0f);
		}

	private:
		void createGeometry()
		{
			const std::array<Eugenix::Render::Vertex::Pos, 4> verts =
			{ {
				{{-1.0f, -1.0f, 0.0f }},
				{{ 0.0f, -1.0f, 1.0f }},
				{{ 1.0f, -1.0f, 0.0f }},
				{{ 0.0f,  1.0f, 0.0f }}
			} };

			const std::array<uint32_t, 12> inds
			{
				0, 3, 1,
				1, 3, 2,
				2, 3, 0,
				0, 1, 2,
			};

			_mesh.Build(std::span{ verts }, std::span{ inds });

			Render::Mesh mesh{};
			mesh.Build(std::span{ verts }, std::span{ inds });

			_meshes.push_back(mesh);
			_meshes.push_back(mesh);
		}

		void createPipelines()
		{
			_program = MakeProgramFromFiles("Shaders/simple_pos_transform_ubo.vert", "Shaders/simple_pos_transform_ubo.frag");
		}

		void createSkybox()
		{
			const std::array<std::string_view, 6> skyboxImages =
			{
				"Textures/Skybox/right.jpg",
				"Textures/Skybox/left.jpg",
				"Textures/Skybox/top.jpg",
				"Textures/Skybox/bottom.jpg",
				"Textures/Skybox/front.jpg",
				"Textures/Skybox/back.jpg"
			};

			skybox.Create(skyboxImages);
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

		Camera _camera{};

		std::vector<Render::Mesh> _meshes;
		Render::Mesh _mesh;

		Render::OpenGL::ShaderProgram _program{};

		Scene::Skybox skybox;

		Math::Transform _transform{};
		Render::Data::Camera _cameraData{};

		Render::OpenGL::Buffer _transformUbo{};
		Render::OpenGL::Buffer _cameraUbo{};
	};
}
