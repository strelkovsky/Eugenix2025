#pragma once

#include <span>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stb_image.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "TestUtils.h"

// Sandbox headers
#include "App/SandboxApp.h"
#include "Assets/ImageLoader.h"
#include "Render/Types.h"
#include "Render/OpenGL/Buffer.h"
#include "Render/OpenGL/Commands.h"
#include "Render/OpenGL/Pipeline.h"
#include "Render/OpenGL/Texture.h"
#include "Render/OpenGL/VertexArray.h"

namespace
{
	class Material
	{
	public:
		Material()
		{
			specularIntensity = 0.0f;
			shininess = 0.0f;
		}

		Material(GLfloat specIntensity, GLfloat shine)
		{
			specularIntensity = specIntensity;
			shininess = shine;
		}

		void UseMaterial(GLint specularIntensityLocation, GLint shininessLocation)
		{
			glUniform1f(specularIntensityLocation, specularIntensity);
			glUniform1f(shininessLocation, shininess);
		}

		~Material()
		{

		}

	private:
		GLfloat specularIntensity;
		GLfloat shininess;
	};


	Eugenix::Assets::ImageLoader _imageLoader;

	class Model
	{
	public:
		Model()
		{
		}

		void LoadModel(const std::string& fileName)
		{
			printf("Loading model '%s'...\n", fileName.c_str());

			Assimp::Importer importer;
			const aiScene* scene = importer.ReadFile(fileName,
				aiProcess_Triangulate |
				aiProcess_FlipUVs |
				aiProcess_GenSmoothNormals |
				aiProcess_JoinIdenticalVertices);

			if (!scene)
			{
				printf("Model '%s' failed to load: '%s'\n", fileName.c_str(), importer.GetErrorString());
				return;
			}

			printf("Loading meshes...\n");

			LoadNode(scene->mRootNode, scene);

			printf("Loading materials...\n");

			LoadMaterials(scene);
		}

		void RenderModel()
		{
			for (size_t i = 0; i < meshList.size(); i++)
			{
				unsigned int materialIndex = meshToTexture[i];

				if (materialIndex < textureList.size() && textureList[materialIndex])
				{
					textureList[materialIndex]->Bind();
				}

				meshList[i]->RenderMesh();
			}
		}

		void ClearModel()
		{
			for (size_t i = 0; i < meshList.size(); i++)
			{
				if (meshList[i])
				{
					delete meshList[i];
					meshList[i] = nullptr;
				}
			}

			for (size_t i = 0; i < textureList.size(); i++)
			{
				if (textureList[i] != nullptr)
				{
					delete textureList[i];
					textureList[i] = nullptr;
				}
			}

			meshList.clear();
			textureList.clear();
			meshToTexture.clear();
		}

		~Model()
		{
			ClearModel();
		}

	private:
		void LoadNode(aiNode* node, const aiScene* scene)
		{
			for (unsigned int i = 0; i < node->mNumMeshes; i++)
			{
				LoadMesh(scene->mMeshes[node->mMeshes[i]], scene);
			}

			for (unsigned int i = 0; i < node->mNumChildren; i++)
			{
				LoadNode(node->mChildren[i], scene);
			}
		}

		void LoadMesh(aiMesh* mesh, const aiScene* scene)
		{
			std::vector<GLfloat> vertices;
			std::vector<unsigned int> indices;

			for (unsigned int i = 0; i < mesh->mNumVertices; i++)
			{
				vertices.insert(vertices.end(), { mesh->mVertices[i].x, mesh->mVertices[i].y , mesh->mVertices[i].z });
				if (mesh->mTextureCoords[0])
				{
					vertices.insert(vertices.end(), { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y });
				}
				else
				{
					vertices.insert(vertices.end(), { 0.0f, 0.0f });
				}
				vertices.insert(vertices.end(), { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z });
			}

			for (unsigned int i = 0; i < mesh->mNumFaces; i++)
			{
				aiFace face = mesh->mFaces[i];
				for (unsigned int j = 0; j < face.mNumIndices; j++)
				{
					indices.push_back(face.mIndices[j]);
				}
			}

			std::vector<Eugenix::Render::Attribute> attributes
			{
				{ 0, 3, Eugenix::Render::DataType::Float, false, 0 },
				{ 1, 2, Eugenix::Render::DataType::Float, false, (sizeof(glm::vec3)) },
				{ 2, 3, Eugenix::Render::DataType::Float, false, (sizeof(glm::vec3) + sizeof(glm::vec2)) },
			};

			Eugenix::SimpleMesh* newMesh = new Eugenix::SimpleMesh(vertices, indices, attributes, 8 * sizeof(float));
			//newMesh->CreateMesh(&vertices[0], &indices[0], (unsigned int)vertices.size(), (unsigned int)indices.size());
			meshList.push_back(newMesh);
			meshToTexture.push_back(mesh->mMaterialIndex);
		}

		void LoadMaterials(const aiScene* scene)
		{
			textureList.resize(scene->mNumMaterials);

			for (unsigned int i = 0; i < scene->mNumMaterials; i++)
			{
				aiMaterial* material = scene->mMaterials[i];

				textureList[i] = nullptr;

				if (material->GetTextureCount(aiTextureType_DIFFUSE))
				{
					aiString path;
					if (material->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS)
					{
						size_t idx = std::string(path.data).rfind("\\");
						std::string filename = std::string(path.data).substr(idx + 1);

						std::string texPath = std::string("Textures/") + filename;

						printf("Texture loaded 'Textures/%s'\n", filename.c_str());

						auto imageData = _imageLoader.Load(texPath.c_str());

						if (imageData.pixels.get() == nullptr)
						{
							printf("Failed to load texture at '%s'\n", texPath.c_str());
							delete textureList[i];
							textureList[i] = nullptr;
						}
						else
						{
							textureList[i] = new Eugenix::Render::OpenGL::Texture2D();
							textureList[i]->Create();
							textureList[i]->Storage(imageData);
							textureList[i]->Update(imageData);
						}
					}
				}

				if (!textureList[i])
				{
					auto imageData = _imageLoader.Load("Textures/plain.png");
					textureList[i] = new Eugenix::Render::OpenGL::Texture2D();
					textureList[i]->Create();
					textureList[i]->Storage(imageData);
					textureList[i]->Update(imageData);
				}
			}
		}

	private:
		std::vector <Eugenix::SimpleMesh*> meshList;
		std::vector <Eugenix::Render::OpenGL::Texture2D*> textureList;
		std::vector <unsigned int> meshToTexture;
	};
}

namespace Eugenix
{
	class LightingApp final : public SandboxApp
	{
	protected:
		bool onInit() override
		{
			CreateGeometry();
			CreatePipelines();

			glEnable(GL_DEPTH_TEST);

			Render::OpenGL::Commands::Clear(135.0f / 255.0f, 206.0f / 255.0f, 235.0f / 255.0f);

			_camera = Camera(glm::vec3(-25.0f, 45.0f, -2.0f), glm::vec3(0.0f, 1.0f, 0.0f), 0.0f, 0.0f, 4.0f, 0.1f);

			auto imageData = _imageLoader.Load("Textures/brick.png");
			_brickTexture.Create();
			_brickTexture.Storage(imageData);
			_brickTexture.Update(imageData);

			imageData = _imageLoader.Load("Textures/brick.png");
			_pyramidTexture.Create();
			_pyramidTexture.Storage(imageData);
			_pyramidTexture.Update(imageData);

			imageData = _imageLoader.Load("Textures/pyramid.png");
			_brickTexture.Create();
			_brickTexture.Storage(imageData);
			_brickTexture.Update(imageData);

			imageData = _imageLoader.Load("Textures/sponza_floor.jpg");
			_sponzaFloorTexture.Create();
			_sponzaFloorTexture.Storage(imageData);
			_sponzaFloorTexture.Update(imageData);

			imageData = _imageLoader.Load("Textures/sponza_wall.jpg");
			_sponzaWallTexture.Create();
			_sponzaWallTexture.Storage(imageData);
			_sponzaWallTexture.Update(imageData);

			imageData = _imageLoader.Load("Textures/crate.png");
			_crateTexture.Create();
			_crateTexture.Storage(imageData);
			_crateTexture.Update(imageData);

			shinyMaterial = Material(1.0f, 128.0f);
			dullMaterial = Material(1.0f, 64.0f);
			superShinyMaterial = Material(1.0f, 256.0f);

			sponza = Model();
			sponza.LoadModel("Models/sponza.obj");

			mainLight = DirectionalLight({ 1.0f, 1.0f, 1.0f }, 0.2f, 1.2f, { 0.76f, -0.64f, -0.1f });

			unsigned int pointLightCount = 0;
			pointLights[0] = PointLight({ 1.0f, 1.0f, 0.9f }, 0.2f, 1.0f, { 4.0f, 2.0f, 2.0f }, 0.3f, 0.2f, 0.1f);
			pointLightCount++;
			pointLights[1] = PointLight({ 0.0f, 1.0f, 0.0f }, 0.1f, 1.0f, { -4.0f, 6.0f, -6.0f }, 0.3f, 0.2f, 0.1f);
			pointLightCount++;
			pointLights[2] = PointLight({ 0.0f, 0.0f, 1.0f }, 0.1f, 1.0f, { 4.0f, 12.0f, -2.0f }, 0.3f, 0.2f, 0.1f);
			pointLightCount++;

			unsigned int spotLightCount = 0;
			spotLights[0] = SpotLight({ 1.0f, 1.0f, 0.8f }, 0.3f, 6.0f, { -50.0f, 54.0f, -1.2f }, { -0.6f, -1.0f, 0.0f }, 0.3f, 0.2f, 0.1f, 45.0f);
			spotLightCount++;
			spotLights[1] = SpotLight({ 0.8f, 0.8f, 1.0f }, 0.3f, 6.0f, { -50.0f, 74.0f, -1.2f }, { -0.6f, -1.0f, 0.0f }, 0.3f, 0.2f, 0.1f, 45.0f);
			spotLightCount++;
			spotLights[2] = SpotLight({ 1.0f, 1.0f, 1.0f }, 0.0f, 4.0f, glm::vec3(), glm::vec3(), 0.4f, 0.3f, 0.2f, 35.0f);
			spotLightCount++;

			Assimp::Importer importer = Assimp::Importer();

			return true;
		}

		void onUpdate(float deltaTime) override
		{
			glm::vec3 lowerLight = _camera.getCameraPosition();
			lowerLight.y -= 0.2f;
			spotLights[2].SetFlash(lowerLight, _camera.getCameraDirection());

			_camera.keyControl(getKeys(), deltaTime);
			_camera.mouseControl(getMouseButtons(), getXChange(), getYChange());
		}

		void onRender() override
		{
			Render::OpenGL::Commands::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Projection matrix
			glm::mat4 projection = glm::perspective(45.0f, (GLfloat)width() / (GLfloat)height(), 0.1f, 200.0f);
			_pipeline.Bind();

			{
				mainLight.UseLight(
					uniformDirectionalLight.uniformColor,
					uniformDirectionalLight.uniformAmbientIntensity,
					uniformDirectionalLight.uniformDiffuseIntensity,
					uniformDirectionalLight.uniformDirection);

				auto lightCount = 3;

				if (lightCount > MAX_POINT_LIGHTS) lightCount = MAX_POINT_LIGHTS;

				glUniform1i(uniformPointLightCount, lightCount);

				for (size_t i = 0; i < lightCount; i++)
				{
					pointLights[i].UseLight(
						uniformPointLight[i].uniformColor,
						uniformPointLight[i].uniformAmbientIntensity,
						uniformPointLight[i].uniformDiffuseIntensity,
						uniformPointLight[i].uniformPosition,
						uniformPointLight[i].uniformConstant,
						uniformPointLight[i].uniformLinear,
						uniformPointLight[i].uniformExponent);
				}

				auto spotLichtCount = 3;

				if (spotLichtCount > MAX_SPOT_LIGHTS) spotLichtCount = MAX_SPOT_LIGHTS;

				glUniform1i(uniformSpotLightCount, spotLichtCount);

				for (size_t i = 0; i < spotLichtCount; i++)
				{
					spotLights[i].UseLight(
						uniformSpotLight[i].uniformColor,
						uniformSpotLight[i].uniformAmbientIntensity,
						uniformSpotLight[i].uniformDiffuseIntensity,
						uniformSpotLight[i].uniformPosition,
						uniformSpotLight[i].uniformDirection,
						uniformSpotLight[i].uniformConstant,
						uniformSpotLight[i].uniformLinear,
						uniformSpotLight[i].uniformExponent,
						uniformSpotLight[i].uniformEdge);
				}

				glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(_camera.CalculateViewMatrix()));
				glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
				glUniform3f(uniformEyePosition, _camera.getCameraPosition().x, _camera.getCameraPosition().y, _camera.getCameraPosition().z);

				// Model matrix
				glm::mat4 model;

				/* Cube Left */
				model = glm::mat4(1.0f);
				model = glm::translate(model, glm::vec3(-9.0f, 1.0f, -9.0f));
				model = glm::rotate(model, 0.0f, glm::vec3(1.0f, 0.0f, 0.0f));
				model = glm::rotate(model, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
				model = glm::rotate(model, 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
				model = glm::scale(model, glm::vec3(2.0f));
				glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
				_brickTexture.Bind();
				shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
				meshes[0].RenderMesh();

				/* Cube Right */
				model = glm::mat4(1.0f);
				model = glm::translate(model, glm::vec3(9.0f, 1.0f, -9.0f));
				model = glm::rotate(model, 0.0f, glm::vec3(1.0f, 0.0f, 0.0f));
				model = glm::rotate(model, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
				model = glm::rotate(model, 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
				model = glm::scale(model, glm::vec3(2.0f));
				glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
				_crateTexture.Bind();
				dullMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
				meshes[1].RenderMesh();

				/* Floor */
				model = glm::mat4(1.0f);
				model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
				model = glm::scale(model, glm::vec3(1.0f));
				glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
				_sponzaFloorTexture.Bind();
				superShinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
				meshes[2].RenderMesh();

				/* Floor 2nd */
				model = glm::mat4(1.0f);
				model = glm::translate(model, glm::vec3(0.0f, 10.0f, 0.0f));
				model = glm::scale(model, glm::vec3(1.0f));
				glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
				_sponzaFloorTexture.Bind();
				superShinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
				meshes[2].RenderMesh();

				/* Floor 3nd */
				model = glm::mat4(1.0f);
				model = glm::translate(model, glm::vec3(0.0f, 20.0f, 0.0f));
				model = glm::scale(model, glm::vec3(1.0f));
				glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
				_sponzaFloorTexture.Bind();
				superShinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
				meshes[2].RenderMesh();

				/* Wall Right */
				model = glm::mat4(1.0f);
				model = glm::translate(model, glm::vec3(10.0f, 10.0f, 0.0f));
				model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
				model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
				model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
				model = glm::scale(model, glm::vec3(1.0f));
				glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
				_sponzaWallTexture.Bind();
				shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
				meshes[3].RenderMesh();

				/* Wall Left */
				model = glm::mat4(1.0f);
				model = glm::translate(model, glm::vec3(-10.0f, 10.0f, 0.0f));
				model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
				model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
				model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
				model = glm::scale(model, glm::vec3(1.0f));
				glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
				_sponzaWallTexture.Bind();
				shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
				meshes[3].RenderMesh();

				/* Wall Back */
				model = glm::mat4(1.0f);
				model = glm::translate(model, glm::vec3(0.0f, 10.0f, -10.0f));
				model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
				model = glm::rotate(model, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
				model = glm::rotate(model, 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
				model = glm::scale(model, glm::vec3(1.0f));
				glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
				_sponzaWallTexture.Bind();
				shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
				meshes[3].RenderMesh();

				/* Ceil */
				model = glm::mat4(1.0f);
				model = glm::translate(model, glm::vec3(0.0f, 9.99f, 0.0f));
				model = glm::rotate(model, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
				model = glm::scale(model, glm::vec3(1.0f));
				glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
				_sponzaCeilTexture.Bind();
				superShinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
				meshes[4].RenderMesh();

				/* Ceil 2nd */
				model = glm::mat4(1.0f);
				model = glm::translate(model, glm::vec3(0.0f, 19.99f, 0.0f));
				model = glm::rotate(model, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
				model = glm::scale(model, glm::vec3(1.0f));
				glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
				_sponzaCeilTexture.Bind();
				superShinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
				meshes[4].RenderMesh();

				/* Sponza scene */
				model = glm::mat4(1.0f);
				model = glm::translate(model, glm::vec3(0.0f, 40.0f, 0.0f));
				model = glm::rotate(model, 0.0f, glm::vec3(1.0f, 0.0f, 0.0f));
				model = glm::rotate(model, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
				model = glm::rotate(model, 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
				model = glm::scale(model, glm::vec3(0.04f));
				glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
				superShinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
				sponza.RenderModel();
			}
		}

	private:
		// The Phong shading approach
		void calcAverageNormals(unsigned int* indices, unsigned int indiceCount,
			GLfloat* vertices, unsigned int verticeCount,
			unsigned int vLength, unsigned int normalOffset)
		{
			for (size_t i = 0; i < indiceCount; i += 3)
			{
				unsigned int in0 = indices[i + 0] * vLength;
				unsigned int in1 = indices[i + 1] * vLength;
				unsigned int in2 = indices[i + 2] * vLength;
				glm::vec3 v1(vertices[in1 + 0] - vertices[in0 + 0], vertices[in1 + 1] - vertices[in0 + 1], vertices[in1 + 2] - vertices[in0 + 2]);
				glm::vec3 v2(vertices[in2 + 0] - vertices[in0 + 0], vertices[in2 + 1] - vertices[in0 + 1], vertices[in2 + 2] - vertices[in0 + 2]);
				glm::vec3 normal = glm::cross(v1, v2);
				normal = glm::normalize(normal);

				in0 += normalOffset;
				in1 += normalOffset;
				in2 += normalOffset;

				vertices[in0 + 0] += normal.x; vertices[in0 + 1] += normal.y; vertices[in0 + 2] += normal.z;
				vertices[in1 + 0] += normal.x; vertices[in1 + 1] += normal.y; vertices[in1 + 2] += normal.z;
				vertices[in2 + 0] += normal.x; vertices[in2 + 1] += normal.y; vertices[in2 + 2] += normal.z;
			}

			for (unsigned int i = 0; i < verticeCount / vLength; i++)
			{
				unsigned int nOffset = i * vLength + normalOffset;
				glm::vec3 vec(vertices[nOffset + 0], vertices[nOffset + 1], vertices[nOffset + 2]);
				vec = glm::normalize(vec);
				vertices[nOffset + 0] = vec.x; vertices[nOffset + 1] = vec.y; vertices[nOffset + 2] = vec.z;
			}
		}

		void CreateGeometry()
		{
			GLfloat vertices[] =
			{
				//  X      Y      Z       U     V       NX     NY     NZ
				-0.5f,  0.5f, -0.5f,   1.0f, 1.0f,   -0.5f,  0.5f, -0.5f,
				-0.5f, -0.5f, -0.5f,   1.0f, 0.0f,   -0.5f, -0.5f, -0.5f,
				 0.5f, -0.5f, -0.5f,   0.0f, 0.0f,    0.5f, -0.5f, -0.5f,
				 0.5f,  0.5f, -0.5f,   0.0f, 1.0f,    0.5f,  0.5f, -0.5f,

				-0.5f,  0.5f,  0.5f,   0.0f, 1.0f,   -0.5f,  0.5f,  0.5f,
				-0.5f, -0.5f,  0.5f,   0.0f, 0.0f,   -0.5f, -0.5f,  0.5f,
				 0.5f, -0.5f,  0.5f,   1.0f, 0.0f,    0.5f, -0.5f,  0.5f,
				 0.5f,  0.5f,  0.5f,   1.0f, 1.0f,    0.5f,  0.5f,  0.5f,

				 0.5f,  0.5f, -0.5f,   1.0f, 1.0f,    0.5f,  0.5f, -0.5f,
				 0.5f, -0.5f, -0.5f,   1.0f, 0.0f,    0.5f, -0.5f, -0.5f,
				 0.5f, -0.5f,  0.5f,   0.0f, 0.0f,    0.5f, -0.5f,  0.5f,
				 0.5f,  0.5f,  0.5f,   0.0f, 1.0f,    0.5f,  0.5f,  0.5f,

				-0.5f,  0.5f, -0.5f,   0.0f, 1.0f,   -0.5f,  0.5f, -0.5f,
				-0.5f, -0.5f, -0.5f,   0.0f, 0.0f,   -0.5f, -0.5f, -0.5f,
				-0.5f, -0.5f,  0.5f,   1.0f, 0.0f,   -0.5f, -0.5f,  0.5f,
				-0.5f,  0.5f,  0.5f,   1.0f, 1.0f,   -0.5f,  0.5f,  0.5f,

				-0.5f,  0.5f,  0.5f,   1.0f, 1.0f,   -0.5f,  0.5f,  0.5f,
				-0.5f,  0.5f, -0.5f,   1.0f, 0.0f,   -0.5f,  0.5f, -0.5f,
				 0.5f,  0.5f, -0.5f,   0.0f, 0.0f,    0.5f,  0.5f, -0.5f,
				 0.5f,  0.5f,  0.5f,   0.0f, 1.0f,    0.5f,  0.5f,  0.5f,

				-0.5f, -0.5f,  0.5f,   0.0f, 1.0f,   -0.5f, -0.5f,  0.5f,
				-0.5f, -0.5f, -0.5f,   0.0f, 0.0f,   -0.5f, -0.5f, -0.5f,
				 0.5f, -0.5f, -0.5f,   1.0f, 0.0f,    0.5f, -0.5f, -0.5f,
				 0.5f, -0.5f,  0.5f,   1.0f, 1.0f,    0.5f, -0.5f,  0.5f,
			};

			unsigned int indices[] =
			{
				 0,  3,  1,
				 3,  2,  1,
				 4,  5,  7,
				 7,  5,  6,
				 8, 11,  9,
				11, 10,  9,
				12, 13, 15,
				15, 13, 14,
				16, 19, 17,
				19, 18, 17,
				20, 21, 23,
				23, 21, 22,
			};

			meshes.reserve(6);

			std::vector<Render::Attribute> attributes
			{
				{ 0, 3, Render::DataType::Float, GL_FALSE,  0 },
				{ 1, 2, Render::DataType::Float, GL_FALSE,  (sizeof(glm::vec3)) },
				{ 2, 3, Render::DataType::Float, GL_FALSE,  (sizeof(glm::vec3) + sizeof(glm::vec2)) },
			};

			calcAverageNormals(indices, 12, vertices, 8 * 4 * 6, 8, 5);

			meshes.emplace_back(vertices, indices, attributes, 8 * sizeof(float));
			meshes.emplace_back(vertices, indices, attributes, 8 * sizeof(float));

			GLfloat floorVertices[] =
			{
				-10.0f, 0.0f, -10.0f,    0.0f,  0.0f,   0.0f, 1.0f, 0.0f,
				 10.0f, 0.0f, -10.0f,   10.0f,  0.0f,   0.0f, 1.0f, 0.0f,
				-10.0f, 0.0f,  10.0f,    0.0f, 10.0f,   0.0f, 1.0f, 0.0f,
				 10.0f, 0.0f,  10.0f,   10.0f, 10.0f,   0.0f, 1.0f, 0.0f,
			};


			unsigned int floorIndices[] =
			{
				0, 2, 1,
				1, 2, 3,
			};

			meshes.emplace_back(floorVertices, floorIndices, attributes, 8 * sizeof(float));
			meshes.emplace_back(floorVertices, floorIndices, attributes, 8 * sizeof(float));
			meshes.emplace_back(floorVertices, floorIndices, attributes, 8 * sizeof(float));
		}

		void CreatePipelines()
		{
			_pipeline.Create();

			const auto vsSourceData = Eugenix::IO::FileContent("Shaders/ambient_shader.vert");
			const char* vsSource = vsSourceData.data();

			const auto fsSourceData = Eugenix::IO::FileContent("Shaders/ambient_shader.frag");
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

			uniformDirectionalLight.uniformColor = glGetUniformLocation(_pipeline.NativeHandle(), "directionalLight.base.color");
			uniformDirectionalLight.uniformAmbientIntensity = glGetUniformLocation(_pipeline.NativeHandle(), "directionalLight.base.ambientIntensity");
			uniformDirectionalLight.uniformDiffuseIntensity = glGetUniformLocation(_pipeline.NativeHandle(), "directionalLight.base.diffuseIntensity");
			uniformDirectionalLight.uniformDirection = glGetUniformLocation(_pipeline.NativeHandle(), "directionalLight.direction");

			uniformEyePosition = glGetUniformLocation(_pipeline.NativeHandle(), "eyePosition");

			uniformSpecularIntensity = glGetUniformLocation(_pipeline.NativeHandle(), "material.specularIntensity");
			uniformShininess = glGetUniformLocation(_pipeline.NativeHandle(), "material.shininess");

			uniformPointLightCount = glGetUniformLocation(_pipeline.NativeHandle(), "pointLightCount");

			for (int i = 0; i < MAX_POINT_LIGHTS; i++)
			{
				char locBuff[100] = { '\0' };

				snprintf(locBuff, sizeof(locBuff), "pointLights[%d].base.color", i);
				uniformPointLight[i].uniformColor = glGetUniformLocation(_pipeline.NativeHandle(), locBuff);

				snprintf(locBuff, sizeof(locBuff), "pointLights[%d].base.ambientIntensity", i);
				uniformPointLight[i].uniformAmbientIntensity = glGetUniformLocation(_pipeline.NativeHandle(), locBuff);

				snprintf(locBuff, sizeof(locBuff), "pointLights[%d].base.diffuseIntensity", i);
				uniformPointLight[i].uniformDiffuseIntensity = glGetUniformLocation(_pipeline.NativeHandle(), locBuff);

				snprintf(locBuff, sizeof(locBuff), "pointLights[%d].position", i);
				uniformPointLight[i].uniformPosition = glGetUniformLocation(_pipeline.NativeHandle(), locBuff);

				snprintf(locBuff, sizeof(locBuff), "pointLights[%d].constant", i);
				uniformPointLight[i].uniformConstant = glGetUniformLocation(_pipeline.NativeHandle(), locBuff);

				snprintf(locBuff, sizeof(locBuff), "pointLights[%d].linear", i);
				uniformPointLight[i].uniformLinear = glGetUniformLocation(_pipeline.NativeHandle(), locBuff);

				snprintf(locBuff, sizeof(locBuff), "pointLights[%d].exponent", i);
				uniformPointLight[i].uniformExponent = glGetUniformLocation(_pipeline.NativeHandle(), locBuff);
			}

			uniformSpotLightCount = glGetUniformLocation(_pipeline.NativeHandle(), "spotLightCount");

			for (int i = 0; i < MAX_SPOT_LIGHTS; i++)
			{
				char locBuff[100] = { '\0' };

				snprintf(locBuff, sizeof(locBuff), "spotLights[%d].base.base.color", i);
				uniformSpotLight[i].uniformColor = glGetUniformLocation(_pipeline.NativeHandle(), locBuff);

				snprintf(locBuff, sizeof(locBuff), "spotLights[%d].base.base.ambientIntensity", i);
				uniformSpotLight[i].uniformAmbientIntensity = glGetUniformLocation(_pipeline.NativeHandle(), locBuff);

				snprintf(locBuff, sizeof(locBuff), "spotLights[%d].base.base.diffuseIntensity", i);
				uniformSpotLight[i].uniformDiffuseIntensity = glGetUniformLocation(_pipeline.NativeHandle(), locBuff);

				snprintf(locBuff, sizeof(locBuff), "spotLights[%d].base.position", i);
				uniformSpotLight[i].uniformPosition = glGetUniformLocation(_pipeline.NativeHandle(), locBuff);

				snprintf(locBuff, sizeof(locBuff), "spotLights[%d].direction", i);
				uniformSpotLight[i].uniformDirection = glGetUniformLocation(_pipeline.NativeHandle(), locBuff);

				snprintf(locBuff, sizeof(locBuff), "spotLights[%d].base.constant", i);
				uniformSpotLight[i].uniformConstant = glGetUniformLocation(_pipeline.NativeHandle(), locBuff);

				snprintf(locBuff, sizeof(locBuff), "spotLights[%d].base.linear", i);
				uniformSpotLight[i].uniformLinear = glGetUniformLocation(_pipeline.NativeHandle(), locBuff);

				snprintf(locBuff, sizeof(locBuff), "spotLights[%d].base.exponent", i);
				uniformSpotLight[i].uniformExponent = glGetUniformLocation(_pipeline.NativeHandle(), locBuff);

				snprintf(locBuff, sizeof(locBuff), "spotLights[%d].edge", i);
				uniformSpotLight[i].uniformEdge = glGetUniformLocation(_pipeline.NativeHandle(), locBuff);
			}
		}

		std::vector<SimpleMesh> meshes;

		Eugenix::Render::OpenGL::Pipeline _pipeline{};

		GLint uniformModel{};
		GLint uniformView{};
		GLint uniformProjection{};
		GLint uniformEyePosition;

		struct
		{
			GLint uniformColor;
			GLint uniformAmbientIntensity;
			GLint uniformDiffuseIntensity;
			GLint uniformDirection;
		} uniformDirectionalLight;

		GLint uniformPointLightCount;

		struct
		{
			GLint uniformColor;
			GLint uniformAmbientIntensity;
			GLint uniformDiffuseIntensity;
			GLint uniformPosition;
			GLint uniformConstant;
			GLint uniformLinear;
			GLint uniformExponent;
		} uniformPointLight[MAX_POINT_LIGHTS];

		GLint uniformSpotLightCount;

		struct
		{
			GLint uniformColor;
			GLint uniformAmbientIntensity;
			GLint uniformDiffuseIntensity;
			GLint uniformPosition;
			GLint uniformDirection;
			GLint uniformConstant;
			GLint uniformLinear;
			GLint uniformExponent;
			GLint uniformEdge;
		} uniformSpotLight[MAX_SPOT_LIGHTS];

		GLint uniformSpecularIntensity{};
		GLint uniformShininess{};

		Eugenix::Camera _camera{};

		Render::OpenGL::Texture2D _brickTexture;
		Render::OpenGL::Texture2D _pyramidTexture;
		Render::OpenGL::Texture2D _sponzaFloorTexture{};
		Render::OpenGL::Texture2D _sponzaWallTexture{};
		Render::OpenGL::Texture2D _sponzaCeilTexture{};
		Render::OpenGL::Texture2D _crateTexture{};

		DirectionalLight mainLight{};
		PointLight pointLights[MAX_POINT_LIGHTS];
		SpotLight spotLights[MAX_SPOT_LIGHTS];

		Material shinyMaterial{};
		Material dullMaterial{};
		Material superShinyMaterial{};

		Model sponza{};
	};
}
