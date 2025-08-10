#pragma once

// Sandbox headers
#include "App/SandboxApp.h"
#include "Assets/ImageLoader.h"
#include "Render/OpenGL/Buffer.h"
#include "Render/OpenGL/Commands.h"
#include "Render/OpenGL/Pipeline.h"
#include "Render/OpenGL/ShaderStage.h"
#include "Render/OpenGL/Texture.h"
#include "Render/OpenGL/VertexArray.h"
#include "Render/Types.h"

// Shaders
const GLchar* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 position;\n"
"layout (location = 1) in vec2 texCoord;\n"
"out vec2 TexCoord;\n"
"void main()\n"
"{\n"
"gl_Position = vec4(position, 1.0);\n"
"TexCoord = texCoord;\n"
"}\0";
const GLchar* fragmentShaderSource = "#version 330 core\n"
"in vec2 TexCoord;\n"
"out vec4 color;\n"
"uniform sampler2D ourTexture;\n"
"void main()\n"
"{\n"
"color = texture(ourTexture, TexCoord);\n"
"}\n\0";

namespace Eugenix
{
    struct Vertex
    {
        glm::vec3 pos;
        glm::vec2 uv;

        // TODO : return span of attributes
    };

    class LearnOpenGLApp final : public SandboxApp
    {
    protected:
        bool OnInit() override
        {
            auto vertexStage = CreateStage(vertexShaderSource, Eugenix::Render::ShaderStageType::Vertex);
            auto fragmentStage = CreateStage(fragmentShaderSource, Eugenix::Render::ShaderStageType::Fragment);

            _pipeline.Create();
            _pipeline
                .AttachStage(vertexStage)
                .AttachStage(fragmentStage)
                .Build();

            _pipeline.SetUniform("ourTexture", 0);

            // Set up vertex data (and buffer(s)) and attribute pointers
            const std::vector<Vertex> vertices =
            {
                // Позиции                // Текстурные координаты
                { {  0.5f,  0.5f, 0.0f }, { 1.0f, 1.0f } },   // Верхний правый
                { {  0.5f, -0.5f, 0.0f }, { 1.0f, 0.0f } },   // Нижний правый
                { { -0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f } },   // Нижний левый
                { { -0.5f,  0.5f, 0.0f }, { 0.0f, 1.0f } }    // Верхний левый
            };

            const std::vector<GLuint> indices = 
            {  // Помните, что мы начинаем с 0!
                0, 1, 3,   // Первый треугольник
                1, 2, 3    // Второй треугольник
            };

            Render::OpenGL::Buffer vbo;
            vbo.Create();
            vbo.Storage(Core::MakeData(std::span{ vertices }));

            Render::OpenGL::Buffer ebo;
            ebo.Create();
            ebo.Storage(Core::MakeData(std::span { indices }));

            constexpr Render::Attribute position_attribute{ 0, 3, Render::DataType::Float, false, 0 };
            constexpr Render::Attribute uv_attribute{ 1, 2, Render::DataType::Float, false, offsetof(Vertex, uv) };

            _vao.Create();
            _vao.AttachVertices(vbo, sizeof(Vertex));
            _vao.AttachIndices(ebo);
            _vao.Attribute(position_attribute);
            _vao.Attribute(uv_attribute);

            const auto img = _imageLoader.Load("Textures/container.jpg");

            _texture.Create();
            _texture.Storage(img);
            _texture.Update(img);

            Render::OpenGL::Commands::Clear(0.2f, 0.3f, 0.3f);

            return true;
        }

        void OnUpdate(float deltaTime) override
        {
            // Update the uniform color
            //GLfloat timeValue = glfwGetTime();
            //_color.y = (sin(timeValue) / 2) + 0.5;
            //_pipeline.SetUniform("ourColor", _color);
        }

        void OnRender() override
        {
            Render::OpenGL::Commands::Clear(GL_COLOR_BUFFER_BIT);

            _pipeline.Bind();
            
            _texture.Bind();
            _vao.Bind();

            Render::OpenGL::Commands::DrawIndexed(Render::PrimitiveType::Triangles, 6, Render::DataType::UInt);
        }

        void OnKeyHandle(int key, int code, int action, int mode) override
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
        }

    private:
        Assets::ImageLoader _imageLoader{};

        Render::OpenGL::Texture2D _texture;

        Render::OpenGL::VertexArray _vao;
        Render::OpenGL::Pipeline _pipeline;

        GLint _vertexColorLocation{};
        glm::vec3 _color{0.0f, 0.0f, 0.0f};

        bool _isLineMode{};
	};
}