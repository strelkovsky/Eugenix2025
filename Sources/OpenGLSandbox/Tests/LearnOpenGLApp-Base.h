#pragma once

#include "LearnOpenGL-Shared.h"

#include "Core/Data.h"

// Sandbox headers
#include "App/SandboxApp.h"
#include "Assets/ImageLoader.h"
#include "Assets/ObjModelLoader.h"
#include "Render/Vertex.h"

namespace Eugenix
{
    // TODO : lights UBO from Eugenix. UBO Test. ShaderEnv stuff
	class LearnOpenGLAppBase : public SandboxApp
	{
	protected:
        bool onInit() override
        {
            initCommonGeometry();
            initCommonShaders();
            initCommonTextures();
            initCommonSamplers();
            initCommonUBOs();

            return true;
        }

        void onKeyHandle(int key, int code, int action, int mode) override
        {
            if (key == GLFW_KEY_TAB && action == GLFW_PRESS)
            {
                _isLineMode = !_isLineMode;

                if (_isLineMode)
                {
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                }
                else
                {
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                }
            }

            if (key == GLFW_KEY_F1 && action == GLFW_PRESS)
            {
                _lockCursor = !_lockCursor;
                glfwSetInputMode(WindowHandle(), GLFW_CURSOR, _lockCursor ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
            }

            if (action == GLFW_PRESS)
                _keys[key] = true;
            else if (action == GLFW_RELEASE)
                _keys[key] = false;
        }

        void onMouseHandle(double xPos, double yPos) override
        {
            if (!_lockCursor)
            {
                _firstMouse = true;
                return;
            }

            if (_firstMouse)
            {
                _lastX = xPos;
                _lastY = yPos;
                _firstMouse = false;
            }

            float xoffset = xPos - _lastX;
            float yoffset = _lastY - yPos; // Обратный порядок вычитания потому что оконные Y-координаты возрастают с верху вниз 

            _lastX = xPos;
            _lastY = yPos;

            _camera.ProcessMouseMovement(xoffset, yoffset);
        }

        void proceedCamera(float deltaTime)
        {
            // Camera controls
            if (_keys[GLFW_KEY_W])
                _camera.ProcessKeyboard(FORWARD, deltaTime);
            if (_keys[GLFW_KEY_S])
                _camera.ProcessKeyboard(BACKWARD, deltaTime);
            if (_keys[GLFW_KEY_A])
                _camera.ProcessKeyboard(LEFT, deltaTime);
            if (_keys[GLFW_KEY_D])
                _camera.ProcessKeyboard(RIGHT, deltaTime);
        }

        void proceedLights(float deltaTime)
        {
            for (auto& light : lights)
            {
                if (light.type == LightType::Point)
                {
                    light.position.x = 1.0f + sin(glfwGetTime()) * 2.0f;
                    light.position.y = sin(glfwGetTime() / 2.0f) * 1.0f;
                }
                else if (light.type == LightType::Spot)
                {
                    light.position = _camera.Position;
                    light.direction = _camera.Front;
                }
            }
        }

		Assets::ImageLoader _imageLoader{};
		Assets::ObjModelLoader _modelLoader{};

		Camera2 _camera{ glm::vec3(0.0f, 0.0f, 4.0f) };
        float _lastX;
        float _lastY;
        bool _firstMouse{ true };
        bool _isLineMode{};
        bool _lockCursor{ true };

        int _selectedLightIndex;

        // Common input
        bool _keys[1024];

        // Common geometry
        Render::OpenGL::VertexArray _cubeVao;
        Render::OpenGL::VertexArray _planeVao;
        Render::OpenGL::VertexArray _lightSourceVao;

        // Common shaders
        Render::OpenGL::ShaderProgram _defaultShader;
        Render::OpenGL::ShaderProgram _lampShader;

        // Common textures
        Render::OpenGL::Texture2D _cubeDiffuseTexture;
        Render::OpenGL::Texture2D _cubeSpecularTexture;
        Render::OpenGL::Texture2D _metalAlbedo;

        // Common samplers
        Render::OpenGL::Sampler _defaultSampler;

        // Common UBOs
        Render::OpenGL::Buffer _cameraUbo{};
        Render::OpenGL::Buffer _transformUbo{};
        Render::OpenGL::Buffer _materialUbo{};

        Render::Data::Camera _cameraData{};
        Math::Transform _transform{};
        glm::vec4 _color{ 1.0f };

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

    private:
        void initCommonGeometry()
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
            _cubeVao.AttachVertices(0, vbo, sizeof(Render::Vertex::PosNormalUV));
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
            _planeVao.AttachVertices(0, planeVbo, sizeof(Render::Vertex::PosNormalUV));
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
            _lightSourceVao.AttachVertices(0, vbo, sizeof(Render::Vertex::PosNormalUV));
            _lightSourceVao.Attribute(position_attribute);
            _lightSourceVao.Attribute(uv_attribute);
        }

        void initCommonShaders()
        {
            _defaultShader = MakeProgramFromFiles("shaders/SimpleVertex.vert", "shaders/SimplePhong.frag");
            _lampShader = MakeProgramFromFiles("shaders/SimpleVertex.vert", "shaders/SimpleUnlit.frag");
        }

        void initCommonTextures()
        {
            auto img = _imageLoader.Load("Textures/container2.png");
            _cubeDiffuseTexture.Create();
            _cubeDiffuseTexture.Upload(img);

            img = _imageLoader.Load("Textures/container2_specular.png");
            _cubeSpecularTexture.Create();
            _cubeSpecularTexture.Upload(img, { .colorSpace = Render::TextureColorSpace::Linear });

            img = _imageLoader.Load("Textures/metal.png");
            _metalAlbedo.Create();
            _metalAlbedo.Upload(img);
        }

        void initCommonSamplers()
        {
            _defaultSampler.Create();
            _defaultSampler.Parameter(Render::TextureParam::WrapS, Render::TextureWrapping::Repeat);
            _defaultSampler.Parameter(Render::TextureParam::WrapT, Render::TextureWrapping::Repeat);
            _defaultSampler.Parameter(Render::TextureParam::MinFilter, Render::TextureFilter::Linear);
            _defaultSampler.Parameter(Render::TextureParam::MagFilter, Render::TextureFilter::Linear);
        }

        void initCommonUBOs()
        {
            // TODO!!!
            // Instead of 3 call glBindBufferBase
            //const std::vector ubo_handles{ transform_ubo.handle(), camera_ubo.handle(), material_ubo.handle() };
            //glBindBuffersBase(GL_UNIFORM_BUFFER, core::buffer::transform, core::buffer::count, ubo_handles.data());
            //opengl::Buffer::bind(ubo_handles);
        }
	};
}