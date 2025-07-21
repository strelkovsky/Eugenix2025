#pragma once

// Sandbox headers
#include "App/SandboxApp.h"
#include "Assets/ImageLoader.h"
#include "Render/OpenGL/Buffer.h"
#include "Render/OpenGL/Commands.h"
#include "Render/OpenGL/Pipeline.h"
#include "Render/OpenGL/ShaderStage.h"
#include "Render/OpenGL/VertexArray.h"
#include "Render/Types.h"

// Shaders
const GLchar* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 position;\n"
"layout (location = 1) in vec3 color;\n"
"out vec3 ourColor;\n"
"void main()\n"
"{\n"
"gl_Position = vec4(position, 1.0);\n"
"ourColor = color;\n"
"}\0";
const GLchar* fragmentShaderSource = "#version 330 core\n"
"out vec4 color;\n"
"uniform vec4 ourColor;\n"
"void main()\n"
"{\n"
"color = ourColor;\n"
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

            // Set up vertex data (and buffer(s)) and attribute pointers
            const std::vector<Vertex> vertices =
            {
                // Positions        
                {{ 0.5f, -0.5f, 0.0f}, {0.0f, 0.0f}},  // Bottom Right 
                {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f}},  // Bottom Left
                {{0.0f,  0.5f, 0.0f},  {0.5f, 1.0f}}   // Top 
            };

            GLfloat texCoords[] = {
    0.0f, 0.0f,  // Нижний левый угол 
    1.0f, 0.0f,  // Нижний правый угол
    0.5f, 1.0f   // Верхняя центральная сторона
            };

            Render::OpenGL::Buffer vbo;
            vbo.Create();
            vbo.Storage(Core::MakeData(std::span{ vertices }));

            constexpr Render::Attribute position_attribute{ 0, 3, GL_FLOAT, GL_FALSE,  0 };
            constexpr Render::Attribute uv_attribute{ 1, 2, GL_FLOAT, GL_FALSE,  offsetof(Vertex, uv) };

            _vao.Create();
            _vao.AttachVertices(vbo, sizeof(Vertex));
            _vao.Attribute(position_attribute);
            _vao.Attribute(uv_attribute);

            auto img = _imageLoader.Load("Textures/dirt.png");

            Render::OpenGL::Commands::Clear(0.2f, 0.3f, 0.3f);

            return true;
        }

        void OnUpdate(float deltaTime) override
        {
            // Update the uniform color
            GLfloat timeValue = glfwGetTime();
            _color.y = (sin(timeValue) / 2) + 0.5;
            _pipeline.SetUniform("ourColor", _color);
        }

        void OnRender() override
        {
            Render::OpenGL::Commands::Clear(GL_COLOR_BUFFER_BIT);

            _pipeline.Bind();
            _vao.Bind();

            Render::OpenGL::Commands::DrawVertices(GL_TRIANGLES, 3);
        }

    private:
        Assets::ImageLoader _imageLoader{};

        Render::OpenGL::VertexArray _vao;
        Render::OpenGL::Pipeline _pipeline;

        GLint _vertexColorLocation{};
        glm::vec3 _color{0.0f, 0.0f, 0.0f};
	};
}