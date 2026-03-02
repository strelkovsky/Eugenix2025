#pragma once

#include "TestUtils.h"

// Sandbox headers
#include "App/SandboxApp.h"
#include "Render/OpenGL/Buffer.h"
#include "Render/OpenGL/Commands.h"
#include "Render/Mesh.h"
#include "Render/OpenGL/ShaderProgram.h"
#include "Render/OpenGL/VertexArray.h"
#include "Render/OpenGL/Pipeline.h"
#include "Render/SharedData.h"
#include "Render/Utils/MeshGenerator.h"

namespace Eugenix
{
	class SimpleCameraApp final : public SandboxApp
	{
	protected:
		bool onInit() override
		{
			Render::OpenGL::Pipeline::Enable(Render::PipelineFeature::DepthTest);
			Render::OpenGL::Commands::Clear(0.2f, 0.0f, 0.2f);
		
			_camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f, 2.0f, 0.1f);

			_cameraData.proj = glm::perspective(45.0f, (float)width() / (float)height(), 0.1f, 100.0f);

			CreateGeometry();
			CreateShaders();
			CreateUBOs();

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
			Render::OpenGL::Commands::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			_cameraUbo.Update(Core::MakeData(&_cameraData));

			//_program.SetUniform("view", _cameraData.view);
			//_program.SetUniform("projection", _cameraData.proj);

			_program.Bind();
			{
				_model = glm::mat4(1.0f);
				_model = glm::translate(_model, glm::vec3(-1.0f, 0.0f, -3.0f));
				_model = glm::rotate(_model, 0.0f, glm::vec3(1.0f, 0.0f, 0.0f));
				_model = glm::rotate(_model, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
				_model = glm::rotate(_model, 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
				_model = glm::scale(_model, glm::vec3(0.5f, 0.5f, 0.75f));
				_transformUbo.Update(Core::MakeData(&_model));
				//_program.SetUniform("model", _model);
				_meshes[0].Bind();
				_meshes[0].Draw();

				_model = glm::mat4(1.0f);
				_model = glm::translate(_model, glm::vec3(1.0f, 0.0f, -3.0f));
				_model = glm::rotate(_model, 0.0f, glm::vec3(1.0f, 0.0f, 0.0f));
				_model = glm::rotate(_model, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
				_model = glm::rotate(_model, 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
				_model = glm::scale(_model, glm::vec3(0.5f, 0.5f, 0.75f));
				//_program.SetUniform("model", _model);
				_transformUbo.Update(Core::MakeData(&_model));
				_meshes[1].Bind();
				_meshes[1].Draw();

				_model = glm::mat4(1.0f);
				_model = glm::translate(_model, glm::vec3(0.0f, 0.0f, -3.0f));
				_model = glm::rotate(_model, 0.0f, glm::vec3(1.0f, 0.0f, 0.0f));
				_model = glm::rotate(_model, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
				_model = glm::rotate(_model, 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
				_model = glm::scale(_model, glm::vec3(0.5f, 0.5f, 0.75f));
				//_program.SetUniform("model", _model);
				_transformUbo.Update(Core::MakeData(&_model));
				_mesh.Bind();
				_mesh.Draw();
			}
		}

	private:
		void CreateGeometry()
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

			auto genMesh = Render::Utils::CreateCube();
			_meshes.push_back(genMesh);
		}

		void CreateShaders()
		{
			_program = MakeProgramFromFiles("Shaders/simple_pos_transform_ubo.vert", "Shaders/simple_pos_transform_ubo.frag");
		}

		void CreateUBOs()
		{
			_cameraUbo.Create();
			_cameraUbo.Storage(Core::MakeData(&_cameraData), GL_DYNAMIC_STORAGE_BIT);
			_cameraUbo.Bind(Render::BufferTarget::UBO, Render::BufferBinding::Camera);

			_transformUbo.Create();
			// TODO : see GP4 (create by size, not by data)
			_transformUbo.Storage(Core::MakeData(&_model), GL_DYNAMIC_STORAGE_BIT);
			_transformUbo.Bind(Render::BufferTarget::UBO, Render::BufferBinding::Transform);

			_materialUbo.Create();
			_materialUbo.Storage(Core::MakeData(&_color), GL_DYNAMIC_STORAGE_BIT);
			_materialUbo.Bind(Render::BufferTarget::UBO, Render::BufferBinding::Material);
		}

		Camera _camera{};

		std::vector<Render::Mesh> _meshes;
		Render::Mesh _mesh;

		Render::OpenGL::ShaderProgram _program{};

		Render::OpenGL::Buffer _cameraUbo{};
		Render::OpenGL::Buffer _transformUbo{};
		Render::OpenGL::Buffer _materialUbo{};

		Render::Data::Camera _cameraData{};
		glm::mat4 _model{ 1.0f };
		glm::vec4 _color{ 1.0f };
	};
}
