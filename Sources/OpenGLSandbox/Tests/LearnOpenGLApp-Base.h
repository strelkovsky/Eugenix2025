#pragma once

#include "LearnOpenGL-Shared.h"

#include "Core/Data.h"

// Sandbox headers
#include "App/SandboxApp.h"
#include "Assets/ImageLoader.h"
#include "Assets/ObjModelLoader.h"
#include "Render/Vertex.h"

namespace
{
    const std::vector<Eugenix::Render::Vertex::PosNormalUV> cubeVertices =
    {
        // Z- (back face)
        {{-0.5f, -0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, {0.0f, 0.0f}},
        {{ 0.5f, -0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, {1.0f, 0.0f}},
        {{ 0.5f,  0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, {1.0f, 1.0f}},
        {{ 0.5f,  0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, {1.0f, 1.0f}},
        {{-0.5f,  0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, {0.0f, 1.0f}},
        {{-0.5f, -0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, {0.0f, 0.0f}},

        // Z+ (front face)
        {{-0.5f, -0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {0.0f, 0.0f}},
        {{ 0.5f, -0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {1.0f, 0.0f}},
        {{ 0.5f,  0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {1.0f, 1.0f}},
        {{ 0.5f,  0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {1.0f, 1.0f}},
        {{-0.5f,  0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {0.0f, 1.0f}},
        {{-0.5f, -0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {0.0f, 0.0f}},

        // X- (left face)
        {{-0.5f,  0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, {1.0f, 0.0f}},
        {{-0.5f,  0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, {1.0f, 1.0f}},
        {{-0.5f, -0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, {0.0f, 1.0f}},
        {{-0.5f, -0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, {0.0f, 1.0f}},
        {{-0.5f, -0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, {0.0f, 0.0f}},
        {{-0.5f,  0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, {1.0f, 0.0f}},

        // X+ (right face)
        {{ 0.5f,  0.5f,  0.5f}, { 1.0f,  0.0f,  0.0f}, {1.0f, 0.0f}},
        {{ 0.5f,  0.5f, -0.5f}, { 1.0f,  0.0f,  0.0f}, {1.0f, 1.0f}},
        {{ 0.5f, -0.5f, -0.5f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 1.0f}},
        {{ 0.5f, -0.5f, -0.5f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 1.0f}},
        {{ 0.5f, -0.5f,  0.5f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 0.0f}},
        {{ 0.5f,  0.5f,  0.5f}, { 1.0f,  0.0f,  0.0f}, {1.0f, 0.0f}},

        // Y- (bottom face)
        {{-0.5f, -0.5f, -0.5f}, { 0.0f, -1.0f,  0.0f}, {0.0f, 1.0f}},
        {{ 0.5f, -0.5f, -0.5f}, { 0.0f, -1.0f,  0.0f}, {1.0f, 1.0f}},
        {{ 0.5f, -0.5f,  0.5f}, { 0.0f, -1.0f,  0.0f}, {1.0f, 0.0f}},
        {{ 0.5f, -0.5f,  0.5f}, { 0.0f, -1.0f,  0.0f}, {1.0f, 0.0f}},
        {{-0.5f, -0.5f,  0.5f}, { 0.0f, -1.0f,  0.0f}, {0.0f, 0.0f}},
        {{-0.5f, -0.5f, -0.5f}, { 0.0f, -1.0f,  0.0f}, {0.0f, 1.0f}},

        // Y+ (top face)
        {{-0.5f,  0.5f, -0.5f}, { 0.0f,  1.0f,  0.0f}, {0.0f, 1.0f}},
        {{ 0.5f,  0.5f, -0.5f}, { 0.0f,  1.0f,  0.0f}, {1.0f, 1.0f}},
        {{ 0.5f,  0.5f,  0.5f}, { 0.0f,  1.0f,  0.0f}, {1.0f, 0.0f}},
        {{ 0.5f,  0.5f,  0.5f}, { 0.0f,  1.0f,  0.0f}, {1.0f, 0.0f}},
        {{-0.5f,  0.5f,  0.5f}, { 0.0f,  1.0f,  0.0f}, {0.0f, 0.0f}},
        {{-0.5f,  0.5f, -0.5f}, { 0.0f,  1.0f,  0.0f}, {0.0f, 1.0f}}
    };

    const std::vector<Eugenix::Render::Vertex::PosNormalUV> planeVertices =
    {
        {{ 5.0f, -0.5f,  5.0f},  {0.0f, 1.0f, 0.0f}, {2.0f, 0.0f}},
        {{-5.0f, -0.5f,  5.0f},  {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
        {{-5.0f, -0.5f, -5.0f},  {0.0f, 1.0f, 0.0f}, {0.0f, 2.0f}},

        {{ 5.0f, -0.5f,  5.0f},  {0.0f, 1.0f, 0.0f}, {2.0f, 0.0f}},
        {{-5.0f, -0.5f, -5.0f},  {0.0f, 1.0f, 0.0f}, {0.0f, 2.0f}},
        {{ 5.0f, -0.5f, -5.0f},  {0.0f, 1.0f, 0.0f}, {2.0f, 2.0f}}
    };

    constexpr Eugenix::Render::Attribute position_attribute{ 0, 3, Eugenix::Render::DataType::Float, false, 0 };
    constexpr Eugenix::Render::Attribute normal_attribute  { 1, 3, Eugenix::Render::DataType::Float, false, offsetof(Eugenix::Render::Vertex::PosNormalUV, normal) };
    constexpr Eugenix::Render::Attribute uv_attribute      { 2, 2, Eugenix::Render::DataType::Float, false, offsetof(Eugenix::Render::Vertex::PosNormalUV, uv) };
}

namespace Eugenix
{
    // TODO : lights UBO from Eugenix. UBO Test. ShaderEnv stuff
	class LearnOpenGLAppBase : public SandboxApp
	{
	protected:
        bool onInit() override
        {
            initCommon();

            return true;
        }

        void initCommon()
        {
            createCube();
            createPlane();
            createLightSource();
        }

        void createCube()
        {
            Render::OpenGL::Buffer vbo;
            vbo.Create();
            vbo.Storage(Core::MakeData(std::span{ cubeVertices }));

            _cubeVao.Create();
            _cubeVao.AttachVertices(vbo, sizeof(Render::Vertex::PosNormalUV));
            _cubeVao.Attribute(position_attribute);
            _cubeVao.Attribute(normal_attribute);
            _cubeVao.Attribute(uv_attribute);
        }

        void createPlane()
        {
            Render::OpenGL::Buffer planeVbo;
            planeVbo.Create();
            planeVbo.Storage(Core::MakeData(std::span{ planeVertices }));

            _planeVao.Create();
            _planeVao.AttachVertices(planeVbo, sizeof(Render::Vertex::PosNormalUV));
            _planeVao.Attribute(position_attribute);
            _planeVao.Attribute(normal_attribute);
            _planeVao.Attribute(uv_attribute);
        }

        void createLightSource()
        {
            Render::OpenGL::Buffer vbo;
            vbo.Create();
            vbo.Storage(Core::MakeData(std::span{ cubeVertices }));

            _lightSourceVao.Create();
            _lightSourceVao.AttachVertices(vbo, sizeof(Render::Vertex::PosNormalUV));
            _lightSourceVao.Attribute(position_attribute);
            _lightSourceVao.Attribute(uv_attribute);
        }

		Assets::ImageLoader _imageLoader{};
		Assets::ObjModelLoader _modelLoader{};

		Camera2 _camera{ glm::vec3(0.0f, 0.0f, 4.0f) };

        int _selectedLightIndex;

        // Shared geometry
        Render::OpenGL::VertexArray _cubeVao;
        Render::OpenGL::VertexArray _planeVao;
        Render::OpenGL::VertexArray _lightSourceVao;

        std::vector<Light> lights =
        {
            {
                .type = LightType::Directional,
                .direction = { -0.2f, -1.0f, -0.3f },
                .ambient = { 0.1f, 0.1f, 0.1f },
                .diffuse = { 0.4f, 0.4f, 0.4f },
                .specular = { 0.5f, 0.5f, 0.5f },
            },
            {
                .type = LightType::Spot,
                .direction = _camera.Front,
                .position = _camera.Position,
                .ambient = { 0.0f, 0.0f, 0.0f },
                .diffuse = { 1.0f, 1.0f, 1.0f },
                .specular = { 1.0f, 1.0f, 1.0f },
                .constant = 1.0f,
                .linear = 0.09f,
                .quadratic = 0.032f,
                .cutOff = glm::cos(glm::radians(12.5f)),
                .outerCutOff = glm::cos(glm::radians(15.0f))
            },
            {
                .type = LightType::Point,
                .position = { 0.7f,  0.2f,  2.0f },
                .ambient = { 0.05f, 0.05f, 0.05f },
                .diffuse = { 0.8f, 0.8f, 0.8f },
                .specular = { 1.0f, 1.0f, 1.0f },
                .constant = 1.0f,
                .linear = 0.09f,
                .quadratic = 0.032f
            },
            {
                .type = LightType::Point,
                .position = { 2.3f, -3.3f, -4.0f },
                .ambient = { 0.05f, 0.05f, 0.05f },
                .diffuse = { 0.0f, 0.0f, 0.0f },
                .specular = { 1.0f, 1.0f, 1.0f },
                .constant = 1.0f,
                .linear = 0.09f,
                .quadratic = 0.032f
            },
            {
                .type = LightType::Point,
                .position = { -4.0f,  2.0f, -12.0f },
                .ambient = { 0.05f, 0.05f, 0.05f },
                .diffuse = { 0.0f, 0.0f, 1.0f },
                .specular = { 1.0f, 1.0f, 1.0f },
                .constant = 1.0f,
                .linear = 0.09f,
                .quadratic = 0.032f
            },
            {
                .type = LightType::Point,
                .position = { 0.0f,  0.0f, -3.0f },
                .ambient = { 0.05f, 0.05f, 0.05f },
                .diffuse = { 1.0f, 0.0f, 0.0f },
                .specular = { 1.0f, 1.0f, 1.0f },
                .constant = 1.0f,
                .linear = 0.09f,
                .quadratic = 0.032f
            },
        };
	};
}