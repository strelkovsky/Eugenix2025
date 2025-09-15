#pragma once

#include <span>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "TestUtils.h"

// Sandbox headers
#include "App/SandboxApp.h"
#include "Render/OpenGL/Buffer.h"
#include "Render/OpenGL/Commands.h"
#include "Render/Mesh.h"
#include "Render/OpenGL/Pipeline.h"
#include "Render/OpenGL/VertexArray.h"
#include "Render/OpenGL/OpenGLTypes.h"
#include "Render/OpenGL/TextureCubemap.h"

#include "Scene/Skybox.h"

namespace Eugenix
{
	class SkyboxApp final : public SandboxApp
	{
	protected:
		bool onInit() override
		{
			CreateGeometry();
			CreatePipelines();

			glEnable(GL_DEPTH_TEST);

			Render::OpenGL::Commands::Clear(0.2f, 0.0f, 0.2f);

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

			_camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f, 2.0f, 0.1f);

			return true;
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

			Render::OpenGL::Commands::DepthMask(true);

			glm::mat4 projection = glm::perspective(45.0f, (GLfloat)width() / (GLfloat)height(), 0.1f, 100.0f);

			glm::mat4 view = glm::mat4(glm::mat3(_camera.CalculateViewMatrix()));
			skybox.render(projection * view);

			_pipeline.SetUniform("view", _camera.CalculateViewMatrix());
			_pipeline.SetUniform("projection", projection);

			_pipeline.Bind();

			{
				glm::mat4 model = glm::mat4(1.0f);

				model = glm::mat4(1.0f);
				model = glm::translate(model, glm::vec3(-0.6f, 0.0f, -3.0f));
				model = glm::rotate(model, 0.0f, glm::vec3(1.0f, 0.0f, 0.0f));
				model = glm::rotate(model, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
				model = glm::rotate(model, 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
				model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.75f));
				_pipeline.SetUniform("model", model);
				_meshes[0].Bind();
				_meshes[0].Draw();

				model = glm::mat4(1.0f);
				model = glm::translate(model, glm::vec3(0.6f, 0.0f, -3.0f));
				model = glm::rotate(model, 0.0f, glm::vec3(1.0f, 0.0f, 0.0f));
				model = glm::rotate(model, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
				model = glm::rotate(model, 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
				model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.75f));
				_pipeline.SetUniform("model", model);
				_meshes[1].Bind();
				_meshes[1].Draw();

				model = glm::mat4(1.0f);
				model = glm::translate(model, glm::vec3(0.0f, 0.0f, -3.0f));
				model = glm::rotate(model, 0.0f, glm::vec3(1.0f, 0.0f, 0.0f));
				model = glm::rotate(model, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
				model = glm::rotate(model, 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
				model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.75f));
				_pipeline.SetUniform("model", model);
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
			_meshes.push_back(mesh);
		}

		void CreatePipelines()
		{
			_pipeline = MakePipelineFromFiles("Shaders/simple_pos_transform.vert", "Shaders/simple_pos_transform.frag");
		}

		Eugenix::Camera _camera{};

		std::vector<Render::Mesh> _meshes;
		Render::Mesh _mesh;

		Eugenix::Render::OpenGL::Pipeline _pipeline{};
		GLuint uniformModel{};
		GLuint uniformView{};
		GLuint uniformProjection{};

		Eugenix::Scene::Skybox skybox;
	};
}
