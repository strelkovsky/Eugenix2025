#pragma once

#include "OGLDevBaseMesh.h"
#include "OGLDevCamera.h"
#include "OGLDevCommon.h"
#include "OGLDevMath.h"

#include "App/SandboxApp.h"
#include "Assets/ImageLoader.h"
#include "Render/OpenGL/Buffer.h"
#include "Render/OpenGL/Commands.h"
#include "Render/OpenGL/ShaderProgram.h"
#include "Render/OpenGL/Texture2D.h"
#include "Render/OpenGL/VertexArray.h"

namespace Eugenix
{
	class OGLDevAppBase final : public SandboxApp
	{
    public:
        OGLDevAppBase() : SandboxApp(1920, 1080) {}

    protected:
        bool onInit() override
        {
            createGeometry();
            createShaderProgram();
            createTextures();

            Render::OpenGL::Commands::Clear(0.5f, 0.5f, 0.5f);
            // когда у вас есть замкнутая поверхность, нужно включить отбраковку граней, чтобы включить
            // только внешние части поверхностей, а не треугольники, которые направлены внутрь.
            // треугольники, направленные вперед, должны быть отрисованы по часовой стрелке (???).
            glEnable(GL_CULL_FACE);
            glFrontFace(GL_CCW);
            glCullFace(GL_BACK);

            glEnable(GL_DEPTH_TEST);

            _mesh.LoadMesh("models/spider/spider.obj");

            return true;
        }

        void onUpdate(float deltaTime) override
        {
            //Scale += deltaTime * DeltaSign;
            //if ((Scale >= 1.5f) || (Scale <= 0.5f))
            //    DeltaSign *= -1.0f;

            AngleInRadians += deltaTime * DeltaSign;
            //if ((AngleInRadians >= 1.5708f) || (AngleInRadians <= -1.5708f))
            //    DeltaSign *= -1.0f;

            //Loc += deltaTime * DeltaSign;
            if ((Loc >= 0.5f) || (Loc <= -0.4f))
                DeltaSign *= -1;

            cubeWorldTransform.SetPosition(0.0f, 0.0f, 0.0f);
            cubeWorldTransform.SetRotation(0.0, AngleInRadians, 0.0);
            cubeWorldTransform.SetScale(Scale);

            PersProjInfo perspInfo = { 60.0f, width(), height(), 0.1f, 100.0f };
            projection = Perspective(perspInfo);

            _cameraController.ProcessKeyboard(WindowHandle(), _camera, deltaTime);
        }

        void onRender() override
        {
            Render::OpenGL::Commands::Viewport(0, 0, width(), height());
            Render::OpenGL::Commands::Clear(Render::ClearFlags::Color | Render::ClearFlags::Depth);

            _shaderProgram.Bind();

            {
                _commonSampler.Bind(0);
                _bricksTexture.Bind(0);

                auto finalMatrix = projection * _camera.GetViewMatrix() * cubeWorldTransform.GetMatrix();
                _shaderProgram.SetUniform("gWorld", finalMatrix);

                _vao.Bind();

                Render::OpenGL::Commands::DrawIndexed(Render::PrimitiveType::Triangles, 36, Render::DataType::UInt);
            }
            {
                WorldTransform& worldTransform = _mesh.GetWorldTransform();

                worldTransform.SetScale(0.01f);
                worldTransform.SetPosition(0.0f, 0.0f, 2.0f);
                worldTransform.Rotate(0.0f, 0.0f, 0.0f);

                glm::mat4 World = worldTransform.GetMatrix();

                auto finalMatrix = projection * _camera.GetViewMatrix() * World;
                _shaderProgram.SetUniform("gWorld", finalMatrix);

                _mesh.Render();
            }
        }

        void onKeyHandle(int key, int code, int action, int mode) override
        {
            if (action == GLFW_PRESS)
            {
               // _camera.ProceedKey(key);
            }
        }

        void onMouseHandle(double xPos, double yPos) override
        {
            //_camera.ProceedMouse(xPos, yPos);
            // TODO : перенести в Update(?) после того, как будет выделен модуль Platform с инпутом.
            _cameraController.ProcessMouse(_camera, xPos, yPos);
        }

    private:
        void createGeometry()
        {
            Vertex Vertices[8];

            glm::vec2 t00 = glm::vec2(0.0f, 0.0f);  // Bottom left
            glm::vec2 t01 = glm::vec2(0.0f, 1.0f);  // Top left
            glm::vec2 t10 = glm::vec2(1.0f, 0.0f);  // Bottom right
            glm::vec2 t11 = glm::vec2(1.0f, 1.0f);  // Top right

            Vertices[0] = Vertex(glm::vec3(0.5f, 0.5f, 0.5f), t00);
            Vertices[1] = Vertex(glm::vec3(-0.5f, 0.5f, -0.5f), t01);
            Vertices[2] = Vertex(glm::vec3(-0.5f, 0.5f, 0.5f), t10);
            Vertices[3] = Vertex(glm::vec3(0.5f, -0.5f, -0.5f), t11);
            Vertices[4] = Vertex(glm::vec3(-0.5f, -0.5f, -0.5f), t00);
            Vertices[5] = Vertex(glm::vec3(0.5f, 0.5f, -0.5f), t10);
            Vertices[6] = Vertex(glm::vec3(0.5f, -0.5f, 0.5f), t01);
            Vertices[7] = Vertex(glm::vec3(-0.5f, -0.5f, 0.5f), t11);

            uint32_t indices[] =
            {
                0, 1, 2,
                1, 3, 4,
                5, 6, 3,
                7, 3, 6,
                2, 4, 7,
                0, 7, 6,
                0, 5, 1,
                1, 5, 3,
                5, 0, 6,
                7, 4, 3,
                2, 1, 4,
                0, 2, 7
            };

            _vbo.Create();
            _vbo.Storage(Core::MakeData(Vertices));

            _ibo.Create();
            _ibo.Storage(Core::MakeData(indices));

            _vao.Create();
            _vao.AttachVertices(_vbo, sizeof(Vertex));
            _vao.AttachIndices(_ibo);
            _vao.Attribute({ 0, 3, Render::DataType::Float, false, 0 });
            _vao.Attribute({ 1, 2, Render::DataType::Float, false, sizeof(glm::vec3) });
        }

        void createShaderProgram()
        {
            const auto vsSource = R"(
                #version 330 core

                layout(location = 0) in vec3 Position;
                layout(location = 1) in vec2 TexCoord;

                uniform mat4 gWorld;

                out vec2 Uv;

                void main()
                {
                    gl_Position = gWorld * vec4(Position, 1.0);
                    Uv = TexCoord;
                }
            )";

            const auto fsSource = R"(
                #version 330 core

                in vec2 Uv;

                uniform sampler2D gSampler;

                out vec4 FragColor;

                void main()
                {
                    FragColor = texture(gSampler, Uv);
                }
            )";
            
            _shaderProgram = MakeShaderProgram(vsSource, fsSource);
        }

        void createTextures()
        {
            _commonSampler.Create();
            _commonSampler.Parameter(Render::TextureParam::MinFilter, Render::TextureFilter::MipMapLinear);
            _commonSampler.Parameter(Render::TextureParam::MagFilter, Render::TextureFilter::Linear);
            _commonSampler.Parameter(Render::TextureParam::WrapS, Render::TextureWrapping::Repeat);
            _commonSampler.Parameter(Render::TextureParam::WrapT, Render::TextureWrapping::Repeat);

            auto img = _imageLoader.Load("Textures/bricks.jpg");
            
            _bricksTexture.Create();
            _bricksTexture.Upload(img);
        }

        Render::OpenGL::Buffer _vbo;
        Render::OpenGL::Buffer _ibo;
        Render::OpenGL::VertexArray _vao;

        Render::OpenGL::ShaderProgram _shaderProgram;
        Render::OpenGL::ShaderProgram _modelShaderProgram;

        Render::OpenGL::Sampler _commonSampler;
        Render::OpenGL::Texture2D _bricksTexture;

        float Scale = 0.5f;
        float AngleInRadians = 0.0f;
        float Loc = 0.0f;
        float DeltaSign = 1.0f;

        Assets::ImageLoader _imageLoader{};

        glm::mat4 projection{ 1.0f };
        // цель view-преобразования - переместить все объекты в world вместе с камерой так,
        // чтобы камера находилась в начале координат и смотрела вдоль положительной оси Z.
        // камера также должна быть параллельна земле.
        //glm::mat4 view{ 1.0f };
        Camera3 _camera;
        CameraController _cameraController;
        //FreeLookCameraController _cameraController;

        WorldTransform cubeWorldTransform;

        BasicMesh _mesh;
	};
}
