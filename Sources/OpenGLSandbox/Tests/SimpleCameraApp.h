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
#include "Render/OpenGL/Pipeline.h"
#include "Render/OpenGL/VertexArray.h"

namespace Eugenix
{
	class SimpleCameraApp final : public SandboxApp
	{
	protected:
		bool OnInit() override
		{
			CreateGeometry();
			CreatePipelines();

			glEnable(GL_DEPTH_TEST);

			Render::OpenGL::Commands::Clear(0.2f, 0.0f, 0.2f);
		
			_camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f, 2.0f, 0.1f);

			return true;
		}

		void OnUpdate(float deltaTime) override
		{
			_camera.keyControl(getKeys(), deltaTime);
			_camera.mouseControl(getMouseButtons(), getXChange(), getYChange());
		}

		void OnRender() override
		{
			Render::OpenGL::Commands::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Projection matrix
			glm::mat4 projection = glm::perspective(45.0f, (GLfloat)Width() / (GLfloat)Height(), 0.1f, 100.0f);
			_pipeline.Bind();

			{
				// Model matrix
				glm::mat4 model = glm::mat4(1.0f);

				model = glm::mat4(1.0f);
				model = glm::translate(model, glm::vec3(-0.6f, 0.0f, -3.0f));
				model = glm::rotate(model, 0.0f, glm::vec3(1.0f, 0.0f, 0.0f));
				model = glm::rotate(model, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
				model = glm::rotate(model, 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
				model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.75f));
				glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
				glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(_camera.CalculateViewMatrix()));
				glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
				_meshes[0].RenderMesh();

				model = glm::mat4(1.0f);
				model = glm::translate(model, glm::vec3(0.6f, 0.0f, -3.0f));
				model = glm::rotate(model, 0.0f, glm::vec3(1.0f, 0.0f, 0.0f));
				model = glm::rotate(model, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
				model = glm::rotate(model, 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
				model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.75f));
				glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
				glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(_camera.CalculateViewMatrix()));
				glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
				_meshes[1].RenderMesh();
			}
		}

	private:
		void CreateGeometry()
		{
			GLfloat vertices[] =
			{
				-1.0f, -1.0f, 0.0f,
				 0.0f, -1.0f, 1.0f,
				 1.0f, -1.0f, 0.0f,
				 0.0f,  1.0f, 0.0f,
			};

			unsigned int indices[] =
			{
				0, 3, 1,
				1, 3, 2,
				2, 3, 0,
				0, 1, 2,
			};

			_meshes.reserve(2);

			std::vector<Render::Attribute> attributes
			{
				{ 0, 3, Render::DataType::Float, false, 0 }
			};

			_meshes.emplace_back(vertices, indices, attributes, 3 * sizeof(float));
			_meshes.emplace_back(vertices, indices, attributes, 3 * sizeof(float));
		}

		void CreatePipelines()
		{
			_pipeline.Create();

			const auto vsSourceData = Eugenix::IO::FileContent("Shaders/shader.vert");
			const char* vsSource = vsSourceData.data();

			const auto fsSourceData = Eugenix::IO::FileContent("Shaders/shader.frag");
			const char* fsSource = fsSourceData.data();

			auto vertexStage = Eugenix::CreateStage(vsSource, Eugenix::Render::ShaderStageType::Vertex);
			auto fragmentStage = Eugenix::CreateStage(fsSource, Eugenix::Render::ShaderStageType::Fragment);

			_pipeline.Create();
			_pipeline.AttachStage(vertexStage)
				.AttachStage(fragmentStage)
				.Build();

			uniformModel = glGetUniformLocation(_pipeline.NativeHandle(), "model");
			uniformView = glGetUniformLocation(_pipeline.NativeHandle(), "view");
			uniformProjection = glGetUniformLocation(_pipeline.NativeHandle(), "projection");
		}

		Eugenix::Camera _camera{};

		std::vector<SimpleMesh> _meshes;

		Eugenix::Render::OpenGL::Pipeline _pipeline{};
		GLuint uniformModel{};
		GLuint uniformView{};
		GLuint uniformProjection{};
	};
}
