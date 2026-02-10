#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>

#include <glm/gtc/random.hpp>

// Sandbox headers
#include "App/SandboxApp.h"
#include "Core/Math/Math.h"
#include "Render/Attribute.h"
#include "Render/OpenGL/Buffer.h"
#include "Render/OpenGL/Pipeline.h"
#include "Render/OpenGL/VertexArray.h"

namespace Game
{
	enum card_state
	{
		visible,
		hidden,
		turned,
		turning
	};

	struct card
	{
		float angle{ };
		bool turning{ };
		bool turned{ };
		bool flipped{ };
		bool reversing{ };

		int32_t type{ };

		vec3 color{ 1.0f };
	};
}

btCollisionWorld* world;

glm::mat4 view;
glm::mat4 proj;

Game::card cards[4][13]{ };

Game::card* last_card{ };

static auto card_is_matching = false;
static auto card_is_turning = false;
static auto card_is_reversing = false;

static constexpr auto card_columns_count = 13;

static auto cursor_x2 = 0.0f;
static auto cursor_y2 = 0.0f;

namespace Eugenix
{
	class Match2App final : public SandboxApp
	{
	public:
		Match2App() : SandboxApp(1920, 980) {}

	protected:
		bool onInit() override
		{
			const auto card_scene = _cardImporter.ReadFile("Models/card.obj", 0);
			const auto card_mesh = card_scene->mMeshes[0];

			for (auto i = 0; i < card_mesh->mNumVertices; i++)
			{
				const auto& vertex = card_mesh->mVertices[i];

				card_vertices.emplace_back(vertex.x, vertex.y, vertex.z);
			}

			for (auto i = 0; i < card_mesh->mNumFaces; i++)
			{
				const auto& face = card_mesh->mFaces[i];

				for (auto j = 0; j < face.mNumIndices; j++)
				{
					card_elements.emplace_back(face.mIndices[j]);
				}
			}

			Render::OpenGL::Buffer _cardVbo;
			_cardVbo.Create();
			_cardVbo.Storage(Core::MakeData(card_vertices));

			_cardEbo.Create();
			_cardEbo.Storage(Core::MakeData(card_elements));

			constexpr Render::Attribute position_attribute{ 0, 3, Render::DataType::Float, false, 0 };

			_cardVao.Create();
			_cardVao.AttachVertices(_cardVbo, sizeof(glm::vec3));
			_cardVao.AttachIndices(_cardEbo);
			_cardVao.Attribute(position_attribute);

			_pipeline = Eugenix::MakePipelineFromFiles("Shaders/simple_pos_transform_ubo.vert", "Shaders/simple_pos_transform_ubo.frag");

			proj = glm::ortho(0.0f, static_cast<float>(width()), 0.0f, static_cast<float>(height()), -1.0f, 1.0f);
			view = glm::mat4(1.0f);

			const std::vector camera_uniforms
			{
				view, proj
			};

			_transformUbo.Create();
			_transformUbo.Storage(Core::MakeData(&_model), GL_DYNAMIC_STORAGE_BIT);
			_transformUbo.Bind(Render::BufferTarget::UBO, Render::BufferBinding::Transform);

			_cameraUbo.Create();
			_cameraUbo.Storage(Core::MakeData(camera_uniforms), GL_DYNAMIC_STORAGE_BIT);
			_cameraUbo.Bind(Render::BufferTarget::UBO, Render::BufferBinding::Camera);

			_materialUbo.Create();
			_materialUbo.Storage(Core::MakeData(&_materialAlbedo), GL_DYNAMIC_STORAGE_BIT);
			_materialUbo.Bind(Render::BufferTarget::UBO, Render::BufferBinding::Material);

			for (auto i = 0; i < 26; i++)
			{
				_cardColors.emplace_back(glm::linearRand(glm::vec3(0.0f), glm::vec3(1.0f)));
			}

			const auto bt_world_configuration = new btDefaultCollisionConfiguration;

			world = new btCollisionWorld(new btCollisionDispatcher(bt_world_configuration), new btDbvtBroadphase(), bt_world_configuration);

			auto card_shape = new btBoxShape(btVector3(65.0f, 97.0f, 0.2f));

			auto card_pairing = false;
			auto card_type = 0;

			for (auto row = 0; row < 4; row++)
			{
				for (auto col = 0; col < card_columns_count; col++)
				{
					const auto x = tile_width_size * col - 6.0f * tile_width_size + static_cast<float>(width()) / 2.0f;
					const auto y = tile_height_size * row - 1.5f * tile_height_size + static_cast<float>(height()) / 2.0f;

					btTransform transform;
					transform.setIdentity();
					transform.setOrigin(btVector3(x, y, 0.0f));

					auto card_object = new btCollisionObject();
					card_object->setCollisionShape(card_shape);
					card_object->setWorldTransform(transform);

					card_object->setUserIndex(row);
					card_object->setUserIndex2(col);

					world->addCollisionObject(card_object);

					cards[row][col].type = card_type;

					if (card_pairing)
					{
						card_pairing = false;
						card_type++;
					}
					else
					{
						card_pairing = true;
					}
				}
			}

			for (auto row = 0; row < 4; row++)
			{
				for (auto col = 0; col < card_columns_count; col++)
				{
					auto new_row = glm::linearRand(0, 3);
					auto new_col = glm::linearRand(0, card_columns_count - 1);

					auto& card = cards[row][col];
					auto& new_card = cards[new_row][new_col];

					const auto type = card.type;
					card.type = new_card.type;
					new_card.type = type;
				}
			}

			for (auto row = 0; row < 4; row++)
			{
				for (auto col = 0; col < card_columns_count; col++)
				{
					auto& card = cards[row][col];

					card.color = _cardColors[card.type];
				}
			}

			Render::OpenGL::Commands::Clear(0.08627450980392157f, 0.3803921568627451f, 0.05490196f);

			glEnable(GL_DEPTH_TEST);
			glEnable(GL_CULL_FACE);

			return true;
		}

		void onUpdate(float deltaTime) override
		{
			for (auto row = 0; row < 4; row++)
			{
				for (auto col = 0; col < card_columns_count; col++)
				{
					auto& card = cards[row][col];
					if (card.flipped)
					{
						continue;
					}

					if (card.turning)
					{
						card.angle += deltaTime * card_rotation_speed;
					}
					else if (card.reversing && !card_is_turning)
					{
						card.angle -= deltaTime * card_rotation_speed;
					}
				}
			}
		}

		void onRender() override
		{
			Render::OpenGL::Commands::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			_pipeline.Bind();
			_cardVao.Bind();
			
			for (auto row = 0; row < 4; row++)
			{
				for (auto col = 0; col < card_columns_count; col++)
				{
					auto& card = cards[row][col];
					if (card.flipped)
					{
						continue;
					}

					_materialUbo.Update(Core::MakeData(&_cardBackgroundColor));

					const auto x = tile_width_size * col - 6.0f * tile_width_size + static_cast<float>(width()) / 2.0f;
					const auto y = tile_height_size * row - 1.5f * tile_height_size + static_cast<float>(height()) / 2.0f;

					_model = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, 0.0f));
					_model = glm::scale(_model, glm::vec3(card_scale, card_scale, 1.0f));

					if (card.turning)
					{
						auto a = glm::smoothstep(0.0f, card_rotation_max_angle, card.angle);

						if (a >= 0.5f)
						{
							_materialUbo.Update(Core::MakeData(&card.color));
						}

						_model = glm::rotate(_model, glm::radians(a * card_rotation_max_angle), glm::vec3(0.0f, 1.0f, 0.0f));

						if (card.angle >= card_rotation_max_angle)
						{
							card.turned = true;
							card.turning = false;

							card_is_turning = false;

							if (card_is_matching)
							{
								card_is_matching = false;

								last_card->flipped = true;
								card.flipped = true;

								last_card = nullptr;
							}
						}
					}
					else if (card.reversing && !card_is_turning)
					{
						auto a = glm::smoothstep(0.0f, card_rotation_max_angle, card.angle);

						if (a <= 0.5f)
						{
							_materialUbo.Update(Core::MakeData(&_cardBackgroundColor));
						}
						else
						{
							_materialUbo.Update(Core::MakeData(&card.color));
						}

						_model = glm::rotate(_model, glm::radians(a * card_rotation_max_angle), glm::vec3(0.0f, 1.0f, 0.0f));

						if (card.angle <= 0.0f)
						{
							card.turned = false;
							card.reversing = false;

							if (last_card != nullptr)
							{
								last_card->turned = false;
								last_card->reversing = false;

								last_card = nullptr;
							}

							card_is_reversing = false;
						}
					}
					else if (card.turned)
					{
						_materialUbo.Update(Core::MakeData(&card.color));
					}

					_transformUbo.Update(Core::MakeData(&_model));

					Render::OpenGL::Commands::DrawIndexed(Render::PrimitiveType::Triangles, card_elements.size(), Render::DataType::UInt);
				}
			}
		}

		virtual void onKeyHandle(int key, int code, int action, int mode) override
		{
			if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
			{
				for (auto row = 0; row < 4; row++)
				{
					for (auto col = 0; col < card_columns_count; col++)
					{
						auto& card = cards[row][col];
						card.turned = false;
						card.turning = false;
						card.reversing = false;
						card.flipped = false;
						card.angle = 0.0f;

						card_is_turning = false;
						card_is_reversing = false;
						card_is_matching = false;
						last_card = nullptr;
					}
				}
			}
		}

		void onMouseHandle(double xPos, double yPos) override
		{
			cursor_x2 = xPos;
			cursor_y2 = yPos;
		}

		void onMouseButtonHandle(int button, int action, int mods) override
		{
			if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
			{
				const glm::vec4 viewport{ 0.0f, 0.0f, (float)width(), (float)height()};

				auto start = glm::unProject(glm::vec3(cursor_x2, height() - cursor_y2, -1.0f), view, proj, viewport);
				auto end = glm::unProject(glm::vec3(cursor_x2, height() - cursor_y2, 1.0f), view, proj, viewport);
				end = start + glm::normalize(end - start) * 1000.0f;

				const btVector3 from(start.x, start.y, start.z);
				const btVector3   to(end.x, end.y, end.z);

				btCollisionWorld::ClosestRayResultCallback result(from, to);
				world->rayTest(from, to, result);

				if (result.hasHit())
				{
					std::cout << "hit" << std::endl;

					const auto row = result.m_collisionObject->getUserIndex();
					const auto col = result.m_collisionObject->getUserIndex2();

					auto& card = cards[row][col];

					if (card_is_turning || card_is_reversing || card.turned || card.turning || card.flipped)
					{
						return;
					}

					card.turning = true;
					card_is_turning = true;

					if (last_card != nullptr)
					{
						if (last_card->type == cards[row][col].type)
						{
							card_is_matching = true;
						}
						else
						{
							card_is_matching = false;
							card_is_reversing = true;

							card.reversing = true;
							last_card->reversing = true;
						}
					}
					else
					{
						last_card = &cards[row][col];
					}
				}
			}
		}

	private:
		Assimp::Importer _cardImporter;

		//Render::OpenGL::Buffer _cardVbo;
		Render::OpenGL::Buffer _cardEbo;
		Render::OpenGL::VertexArray _cardVao;

		Render::OpenGL::Pipeline _pipeline;

		glm::mat4 _model{ 1.0f };
		glm::vec3 _materialAlbedo{ 1.0f, 0.0f, 0.0f };

		// UBOs
		Render::OpenGL::Buffer _transformUbo;
		Render::OpenGL::Buffer _cameraUbo;
		Render::OpenGL::Buffer _materialUbo;

		std::vector<glm::vec3> card_vertices;
		std::vector<uint32_t>  card_elements;

		glm::vec3 _cardBackgroundColor{ 0.97647058f, 0.47843137254901963f, 0.0f };
		std::vector<glm::vec3> _cardColors;

		float tile_width_size = 145.5f;
		float tile_height_size = 212.0f;

		float card_rotation_speed = 180.0f;
		float card_rotation_max_angle = 180.0f;
		float card_scale = 130.0f;
	};
}