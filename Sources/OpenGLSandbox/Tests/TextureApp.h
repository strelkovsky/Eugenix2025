#pragma once

#include <span>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stb_image.h>

#include "TestUtils.h"

// Sandbox headers
#include "App/SandboxApp.h"
#include "Render/OpenGL/Buffer.h"
#include "Render/OpenGL/Commands.h"
#include "Render/OpenGL/Pipeline.h"
#include "Render/OpenGL/VertexArray.h"

namespace
{
	class Texture
	{
	public:
		Texture()
		{
			textureID = 0;
			width = 0;
			height = 0;
			bitDepth = 0;
			fileLocation = "";
		}
		Texture(const char* fileLoc)
			: Texture()
		{
			fileLocation = fileLoc;
		}

		void LoadTexture()
		{
			unsigned char* texData = stbi_load(fileLocation, &width, &height, &bitDepth, 0);
			if (!texData)
			{
				printf("Failed to find: '%s'\n", fileLocation);
				return;
			}

			glGenTextures(1, &textureID);
			glBindTexture(GL_TEXTURE_2D, textureID);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData);
			glGenerateMipmap(GL_TEXTURE_2D);

			glBindTexture(GL_TEXTURE_2D, 0);
			stbi_image_free(texData);
		}

		void UseTexture()
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, textureID);
		}

		void ClearTexture()
		{
			glDeleteTextures(1, &textureID);
			textureID = 0;
			width = 0;
			height = 0;
			bitDepth = 0;
			fileLocation = "";
		}

		~Texture()
		{
			ClearTexture();
		}

	private:
		GLuint textureID;
		int width;
		int height;
		int bitDepth;

		const char* fileLocation;

	};
}

namespace Eugenix
{
	class TextureApp final : public SandboxApp
	{
	protected:
		bool OnInit() override
		{
			CreateGeometry();
			CreatePipelines();

			glEnable(GL_DEPTH_TEST);

			Render::OpenGL::Commands::Clear(0.2f, 0.0f, 0.2f);

			_camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f, 2.0f, 0.1f);

			brickTexture = Texture("Textures/brick.png");
			brickTexture.LoadTexture();
			dirtTexture = Texture("Textures/dirt.png");
			dirtTexture.LoadTexture();

			return true;
		}

		void OnUpdate(float deltaTime) override
		{
			_camera.keyControl(getKeys(), deltaTime);
			_camera.mouseControl(getXChange(), getYChange());
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
				brickTexture.UseTexture();
				meshes[0].RenderMesh();

				model = glm::mat4(1.0f);
				model = glm::translate(model, glm::vec3(0.6f, 0.0f, -3.0f));
				model = glm::rotate(model, 0.0f, glm::vec3(1.0f, 0.0f, 0.0f));
				model = glm::rotate(model, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
				model = glm::rotate(model, 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
				model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.75f));
				glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
				glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(_camera.CalculateViewMatrix()));
				glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
				dirtTexture.UseTexture();
				meshes[1].RenderMesh();
			}
		}

	private:
		void CreateGeometry()
		{
			GLfloat vertices[] =
			{
				//  X      Y     Z      U     V
				-1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
				 0.0f, -1.0f, 1.0f,  0.5f, 0.0f,
				 1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
				 0.0f,  1.0f, 0.0f,  0.5f, 1.0f,
			};

			unsigned int indices[] =
			{
				0, 3, 1,
				1, 3, 2,
				2, 3, 0,
				0, 1, 2,
			};

			meshes.reserve(2);

			std::vector<Render::Attribute> attributes
			{
				{ 0, 3, GL_FLOAT, GL_FALSE,  0 },
				{ 1, 2, GL_FLOAT, GL_FALSE,  (sizeof(glm::vec3)) }
			};

			meshes.emplace_back(vertices, indices, attributes, 5 * sizeof(float));
			meshes.emplace_back(vertices, indices, attributes, 5 * sizeof(float));
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

			uniformModel = glGetUniformLocation(_pipeline.NativeHandle(), "model");
			uniformView = glGetUniformLocation(_pipeline.NativeHandle(), "view");
			uniformProjection = glGetUniformLocation(_pipeline.NativeHandle(), "projection");
		}

		std::vector<SimpleMesh> meshes;

		Eugenix::Render::OpenGL::Pipeline _pipeline{};
		GLuint uniformModel{};
		GLuint uniformView{};
		GLuint uniformProjection{};

		Eugenix::Camera _camera{};

		Texture brickTexture{};
		Texture dirtTexture{};
	};
}
