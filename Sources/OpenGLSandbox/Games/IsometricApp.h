#pragma once

// Sandbox headers
#include "App/SandboxApp.h"
#include "Assets/ImageLoader.h"
#include "Render/OpenGL/Pipeline.h"
#include "Render/SharedData.h"

#include "../Tests/TestUtils.h"

#include "IsometricAppStuff.h"

namespace Eugenix
{
    class IsometricApp final : public SandboxApp
    {
    public:
        IsometricApp() : SandboxApp(1920, 1080) {}

    protected:
        bool onInit() override
        {
            _baseTextureShader = MakePipelineFromFiles("Shaders/SimpleSprite.vert", "Shaders/SimpleSprite2.frag");

            auto ground1image = _imageLoader.Load("Textures/Isometric/dirt.png");
            _ground1Texture.Create();
            _ground1Texture.Upload(ground1image);

            auto water1image = _imageLoader.Load("Textures/Isometric/snow.png");
            _water1Texture.Create();
            _water1Texture.Upload(water1image);

            _tileSampler.Create();
            _tileSampler.Parameter(Render::TextureParam::MinFilter, Render::TextureFilter::Nearest);
            _tileSampler.Parameter(Render::TextureParam::MagFilter, Render::TextureFilter::Nearest);

            const auto tile_width = static_cast<float>(ground1image.width);
            const auto tile_height = static_cast<float>(ground1image.height);

            const auto half_tile_width = tile_width / 2.0f;
            const auto half_tile_height = tile_height / 2.0f;

            const std::vector tile_vertices
            {
                -half_tile_width, -half_tile_height, 0.0f, 0.0f,
                 half_tile_width, -half_tile_height, 1.0f, 0.0f,
                -half_tile_width,  half_tile_height, 0.0f, 1.0f,
                 half_tile_width,  half_tile_height, 1.0f, 1.0f
            };

            const std::vector tile_elements
            {
                0, 1, 2,
                3, 2, 1
            };

            Render::OpenGL::Buffer tile_vbo;
            tile_vbo.Create();
            tile_vbo.Storage(Core::MakeData(tile_vertices));

            Render::OpenGL::Buffer tile_ebo;
            tile_ebo.Create();
            tile_ebo.Storage(Core::MakeData(tile_elements));

            // TODO : try to parse attributes from shader
            constexpr Eugenix::Render::Attribute position_attribute{ 0, 2, Eugenix::Render::DataType::Float, GL_FALSE, 0 };
            constexpr Eugenix::Render::Attribute texcoord_attribute{ 1, 2, Eugenix::Render::DataType::Float, GL_FALSE, sizeof(glm::vec2) };

            _tileVao.Create();
            _tileVao.AttachVertices(tile_vbo, sizeof(glm::vec2) + sizeof(glm::vec2));
            _tileVao.AttachIndices(tile_ebo);
            _tileVao.Attribute(position_attribute);
            _tileVao.Attribute(texcoord_attribute);

            _cameraPosition = glm::vec3{ static_cast<float>(width()) / 2.0f, static_cast<float>(height()) / 2.0f, 0.0f };

            _cameraData.view = glm::translate(glm::mat4{ 1.0f }, _cameraPosition);
            _cameraData.proj = glm::ortho(0.0f, static_cast<float>(width()), static_cast<float>(height()), 0.0f);

            _transformUbo.Create();
            _transformUbo.Storage(Core::MakeData(&_model), GL_DYNAMIC_STORAGE_BIT);
            _transformUbo.Bind(Render::BufferTarget::UBO, Render::BufferBinding::Transform);

            _cameraUbo.Create();
            _cameraUbo.Storage(Core::MakeData(&_cameraData), GL_DYNAMIC_STORAGE_BIT);
            _cameraUbo.Bind(Render::BufferTarget::UBO, Render::BufferBinding::Camera);

            _map.Generate(half_tile_width, half_tile_height);
            _map.GenerateCorners();

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            Render::OpenGL::Commands::Clear(0.5f, 0.5f, 0.5f);

            return true;
        }

        void onUpdate(float deltaTime) override
        {
            if (glfwGetKey(WindowHandle(), GLFW_KEY_A) == GLFW_PRESS)
            {
                _cameraPosition.x += _cameraSpeed * deltaTime;
            }
            if (glfwGetKey(WindowHandle(), GLFW_KEY_D) == GLFW_PRESS)
            {
                _cameraPosition.x -= _cameraSpeed * deltaTime;
            }
            if (glfwGetKey(WindowHandle(), GLFW_KEY_W) == GLFW_PRESS)
            {
                _cameraPosition.y += _cameraSpeed * deltaTime;
            }
            if (glfwGetKey(WindowHandle(), GLFW_KEY_S) == GLFW_PRESS)
            {
                _cameraPosition.y -= _cameraSpeed * deltaTime;
            }

            _cameraData.view = glm::translate(glm::mat4(1.0f), _cameraPosition);
            _cameraData.proj = glm::ortho(0.0f, static_cast<float>(width()), static_cast<float>(height()), 0.0f);
            _cameraUbo.Update(Core::MakeData(&_cameraData));
        }

        void onRender() override
        {
            Render::OpenGL::Commands::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            _baseTextureShader.Bind();
            _tileSampler.Bind(/*core::texture::albedo*/0);
            _tileVao.Bind();

            for (auto row = 0; row < tile_rows; row++)
            {
                for (auto col = 0; col < tile_cols; col++)
                {
                    auto& map_tile = _map.tiles[row][col];

                    if (map_tile.type == Game::Ground)
                    {
                        _ground1Texture.Bind(/*core::texture::albedo*/0);
                    }
                    else if (map_tile.type == Game::Water)
                    {
                        _water1Texture.Bind(/*core::texture::albedo*/0);
                    }

                    _model = glm::translate(glm::mat4(1.0f), glm::vec3(_map.tiles[row][col].position, 1.0f));
                    _transformUbo.Update(Core::MakeData(&_model));
                    Render::OpenGL::Commands::DrawIndexed(Render::PrimitiveType::Triangles, /*tile_elements.elements.size()*/6, Render::DataType::UInt);
                }
            }
        }

    private:
        Assets::ImageLoader _imageLoader{};

        Render::OpenGL::Pipeline _baseTextureShader;
        Render::OpenGL::Texture2D _ground1Texture;
        Render::OpenGL::Texture2D _water1Texture;
        Render::OpenGL::Sampler _tileSampler;
        Render::OpenGL::VertexArray _tileVao;

        // UBOs
        Render::OpenGL::Buffer _transformUbo;
        Render::OpenGL::Buffer _cameraUbo;

        Render::Data::Camera _cameraData;
        glm::mat4 _model{ 1.0f };
        glm::vec3 _cameraPosition{ 0.0f };

        int tile_rows = 6;
        int tile_cols = 6;

        Game::Map _map;

        float _cameraSpeed{ 125.0f };
    };
} // namespace Eugenix