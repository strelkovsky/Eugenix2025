#pragma once

#include <imgui/imgui.h>

// Sandbox headers
#include "Render/OpenGL/Buffer.h"
#include "Render/OpenGL/Commands.h"
#include "Render/OpenGL/Pipeline.h"
#include "Render/OpenGL/ShaderStage.h"
#include "Render/OpenGL/Texture2D.h"
#include "Render/OpenGL/VertexArray.h"
#include "Render/Types.h"

#include "LearnOpenGL-Shared.h"
#include "LearnOpenGLApp-Base.h"

namespace Eugenix
{
    int selectedLightIndex;

    struct Framebuffer
    {
        void Create(int width, int height, GLint internalFormat, GLenum format)
        {
            glGenFramebuffers(1, &_fboId);
            glBindFramebuffer(GL_FRAMEBUFFER, _fboId);

            glGenTextures(1, &_colorBuffer);
            glBindTexture(GL_TEXTURE_2D, _colorBuffer);
            GLenum type = (internalFormat == GL_RGBA16F) ? GL_FLOAT : GL_UNSIGNED_BYTE;
            glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glBindTexture(GL_TEXTURE_2D, 0);

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _colorBuffer, 0);

            glGenRenderbuffers(1, &_depthStencilBuffer);
            glBindRenderbuffer(GL_RENDERBUFFER, _depthStencilBuffer);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
            glBindRenderbuffer(GL_RENDERBUFFER, 0);

            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _depthStencilBuffer);

            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            {
                std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
            }

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        void Bind()
        {
            glBindFramebuffer(GL_FRAMEBUFFER, _fboId);
        }

        void Resize(int width, int height)
        {
            glBindTexture(GL_TEXTURE_2D, _colorBuffer);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
            glBindRenderbuffer(GL_RENDERBUFFER, _depthStencilBuffer);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
        }

        void BindColorAttachment()
        {
            glBindTexture(GL_TEXTURE_2D, _colorBuffer);
        }

    private:
        GLuint _fboId{};
        GLuint _colorBuffer{};
        GLuint _depthStencilBuffer{};
    };

    class LearnOpenGLApp final : public LearnOpenGLAppBase
    {
    protected:

        Framebuffer simpleFramebuffer{};
        Framebuffer hdrFramebuffer{};

        void setupFrameBuffers()
        {
            simpleFramebuffer.Create(width(), height(), GL_RGB, GL_RGB);
            hdrFramebuffer.Create(width(), height(), GL_RGBA16F, GL_RGBA);
        }

        bool onInit() override
        {
            setupFrameBuffers();

            _program = MakeProgramFromFiles("shaders/SimpleVertex.vert", "shaders/SimplePhong.frag");
            _lampProgram = MakeProgramFromFiles("shaders/SimpleVertex.vert", "shaders/SimpleUnlit.frag");
            _screenProgram = MakeProgramFromFiles("shaders/quad.vert", "shaders/SimpleSampler.frag");
            _colorInverseProgram = MakeProgramFromFiles("shaders/quad.vert", "shaders/SimpleColorInverse.frag");
            _grayScaleProgram = MakeProgramFromFiles("shaders/quad.vert", "shaders/SimpleGrayScale.frag");
            _kernelEffectProgram = MakeProgramFromFiles("shaders/quad.vert", "shaders/SimpleKernelEffect.frag");
            _blurEffectProgram = MakeProgramFromFiles("shaders/quad.vert", "shaders/SimpleBlur.frag");
            _tonemapPipeline = MakeProgramFromFiles("shaders/quad.vert", "shaders/SimpleTonemap.frag");

            // Set up vertex data (and buffer(s)) and attribute pointers
            const std::vector<Render::Vertex::PosNormalUV> vertices =
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

            const std::vector<Render::Vertex::PosNormalUV> planeVertices = {
                {{-5.0f, -0.5f,  5.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
                {{ 5.0f, -0.5f,  5.0f}, {0.0f, 1.0f, 0.0f}, {2.0f, 0.0f}},
                {{-5.0f, -0.5f, -5.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 2.0f}},

                {{ 5.0f, -0.5f,  5.0f}, {0.0f, 1.0f, 0.0f}, {2.0f, 0.0f}},
                {{ 5.0f, -0.5f, -5.0f}, {0.0f, 1.0f, 0.0f}, {2.0f, 2.0f}},
                {{-5.0f, -0.5f, -5.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 2.0f}}
            };

            const std::vector<Render::Vertex::Sprite> quadVertices =
            { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
                // positions   // texCoords
                {{-1.0f,  1.0f},  {0.0f, 1.0f}},
                {{-1.0f, -1.0f},  {0.0f, 0.0f}},
                {{ 1.0f, -1.0f},  {1.0f, 0.0f}},

                {{-1.0f,  1.0f},  {0.0f, 1.0f}},
                {{ 1.0f, -1.0f},  {1.0f, 0.0f}},
                {{ 1.0f,  1.0f},  {1.0f, 1.0f}}
            };

            const std::vector<Render::Vertex::Sprite> quadVertices2 =
            { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates. NOTE that this plane is now much smaller and at the top of the screen
                // positions   // texCoords
                {{-0.3f,  1.0f},  {0.0f, 1.0f}},
                {{-0.3f,  0.7f},  {0.0f, 0.0f}},
                {{ 0.3f,  0.7f},  {1.0f, 0.0f}},

                {{-0.3f,  1.0f},  {0.0f, 1.0f}},
                {{ 0.3f,  0.7f},  {1.0f, 0.0f}},
                {{ 0.3f,  1.0f},  {1.0f, 1.0f}}
            };

            Render::OpenGL::Buffer vbo;
            vbo.Create();
            vbo.Storage(Core::MakeData(std::span{ vertices }));

            constexpr Render::Attribute position_attribute{ 0, 3, Render::DataType::Float, false, 0 };
            constexpr Render::Attribute normal_attribute{ 1, 3, Render::DataType::Float, false, offsetof(Render::Vertex::PosNormalUV, normal) };
            constexpr Render::Attribute uv_attribute{ 2, 2, Render::DataType::Float, false, offsetof(Render::Vertex::PosNormalUV, uv) };

            _cubeVao.Create();
            _cubeVao.AttachVertices(vbo, sizeof(Render::Vertex::PosNormalUV));
            //_cubeVao.AttachIndices(ebo);
            _cubeVao.Attribute(position_attribute);
            _cubeVao.Attribute(normal_attribute);
            _cubeVao.Attribute(uv_attribute);

            _lightSourceVao.Create();
            _lightSourceVao.AttachVertices(vbo, sizeof(Render::Vertex::PosNormalUV));
            _lightSourceVao.Attribute(position_attribute);
            _lightSourceVao.Attribute(uv_attribute);

            Render::OpenGL::Buffer planeVbo;
            planeVbo.Create();
            planeVbo.Storage(Core::MakeData(std::span{ planeVertices }));

            _planeVao.Create();
            _planeVao.AttachVertices(planeVbo, sizeof(Render::Vertex::PosNormalUV));
            _planeVao.Attribute(position_attribute);
            _planeVao.Attribute(normal_attribute);
            _planeVao.Attribute(uv_attribute);

            // screen quad VAO
            Render::OpenGL::Buffer quadVbo;
            quadVbo.Create();
            quadVbo.Storage(Core::MakeData(std::span{ quadVertices }));

            _quadVao.Create();
            _quadVao.AttachVertices(quadVbo, sizeof(Render::Vertex::Sprite));
            _quadVao.Attribute({ 0, 2, Render::DataType::Float, false, 0 });
            _quadVao.Attribute({ 1, 2, Render::DataType::Float, false, offsetof(Render::Vertex::Sprite, uv) });

            // screen quad 2 VAO
            Render::OpenGL::Buffer quad2Vbo;
            quad2Vbo.Create();
            quad2Vbo.Storage(Core::MakeData(std::span{ quadVertices2 }));

            _quad2Vao.Create();
            _quad2Vao.AttachVertices(quad2Vbo, sizeof(Render::Vertex::Sprite));
            _quad2Vao.Attribute({ 0, 2, Render::DataType::Float, false, 0 });
            _quad2Vao.Attribute({ 1, 2, Render::DataType::Float, false, offsetof(Render::Vertex::Sprite, uv) });

            //unsigned int quadVAO, quadVBO;
            //glGenVertexArrays(1, &quadVAO);
            //glGenBuffers(1, &quadVBO);
            //glBindVertexArray(quadVAO);
            //glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
            //glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
            //glEnableVertexAttribArray(0);
            //glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
            //glEnableVertexAttribArray(1);
            //glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

            auto img = _imageLoader.Load("Textures/container2.png");
            _cubeDiffuseTexture.Create();
            _cubeDiffuseTexture.Upload(img);

            img = _imageLoader.Load("Textures/container2_specular.png");
            _cubeSpecularTexture.Create();
            _cubeSpecularTexture.Upload(img, { .colorSpace = Render::TextureColorSpace::Linear });

            img = _imageLoader.Load("Textures/metal.png");
            _metalAlbedo.Create();
            _metalAlbedo.Upload(img);

            Render::OpenGL::Commands::Clear(0.1f, 0.1f, 0.1f);

            Render::OpenGL::Pipeline::Enable(Render::PipelineFeature::DepthTest);

            glFrontFace(GL_CCW);        // лицевые треугольники Ц против часовой стрелки
            glCullFace(GL_BACK);        // отбрасывать задние

            glfwSetInputMode(WindowHandle(), GLFW_CURSOR, _lockCursor ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);

            lastX = (float)width() / 2.0f, lastY = (float)height() / 2.0f;

            _model = _modelLoader.Load("Models/nanosuit/nanosuit.obj");

            return true;
        }

        void onUpdate(float deltaTime) override
        {
            // Camera controls
            if (keys[GLFW_KEY_W])
                _camera.ProcessKeyboard(FORWARD, deltaTime);
            if (keys[GLFW_KEY_S])
                _camera.ProcessKeyboard(BACKWARD, deltaTime);
            if (keys[GLFW_KEY_A])
                _camera.ProcessKeyboard(LEFT, deltaTime);
            if (keys[GLFW_KEY_D])
                _camera.ProcessKeyboard(RIGHT, deltaTime);

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

        int _selectedPipeline = 0;

        void onDebugUI() override
        {
            //static float exposure = 1.0f;
            //ImGui::SliderFloat("Exposure", &exposure, 0.1f, 5.0f);
            //_tonemapPipeline.SetUniform("exposure", exposure);

            ImGui::Begin("Select FB pipeline");

            ImGui::Text("Selected - %d\n1 - Simple\n2 - ColorInverse\n3 - GrayScale\n4 - Kernel\n5 - Blur", _selectedPipeline + 1);

            //ImGui::DragInt("Pipeline: ", &_selectedPipeline, 0.2f, 0, 5);

            ImGui::End();
        }

        void onResize() override
        {
            simpleFramebuffer.Resize(width(), height());
            hdrFramebuffer.Resize(width(), height());
        }

        void onRender() override
        {
            // Main Pass
            simpleFramebuffer.Bind();
            //hdrFramebuffer.Bind();
            Render::OpenGL::Pipeline::Enable(Render::PipelineFeature::DepthTest);
            Render::OpenGL::Commands::Viewport(0, 0, width(), height());
            Render::OpenGL::Commands::Clear(Render::ClearFlags::Color | Render::ClearFlags::Depth);
            drawScene();

            // второй проход
            glBindFramebuffer(GL_FRAMEBUFFER, 0); // возвращаем буфер кадра по умолчанию
            Render::OpenGL::Commands::Clear(0.1f, 0.1, 0.1f);
            Render::OpenGL::Commands::Clear(Render::ClearFlags::Color);

            Render::OpenGL::ShaderProgram fbProgram{};

            if (_selectedPipeline == 0)
                fbProgram = _screenProgram;
            else if (_selectedPipeline == 1)
                fbProgram = _colorInverseProgram;
            else if (_selectedPipeline == 2)
                fbProgram = _grayScaleProgram;
            else if (_selectedPipeline == 3)
                fbProgram = _kernelEffectProgram;
            else if (_selectedPipeline == 4)
                fbProgram = _blurEffectProgram;

            fbProgram.Bind();
            //_tonemapPipeline.Bind();
            //_tonemapPipeline.SetUniform("screenTexture", 0); // <-- об€зательно!

            Render::OpenGL::Pipeline::Disable(Render::PipelineFeature::DepthTest);
            glActiveTexture(GL_TEXTURE0);
            //hdrFramebuffer.BindColorAttachment();
            simpleFramebuffer.BindColorAttachment();

            // General quad
            _quadVao.Bind();
            Render::OpenGL::Commands::DrawVertices(Render::PrimitiveType::Triangles, 6);

            // Small quad
            _quad2Vao.Bind();
            Render::OpenGL::Commands::DrawVertices(Render::PrimitiveType::Triangles, 6);
        }

        void drawScene()
        {
            glm::mat4 projection{ 1.0f };

            // rotate camera around (0, 0, 0)
            //GLfloat radius = 10.0f;
            //GLfloat camX = sin(glfwGetTime()) * radius;
            //GLfloat camZ = cos(glfwGetTime()) * radius;
            //view = glm::lookAt(glm::vec3(camX, 0.0, camZ), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));

            _program.Bind();

            glm::mat4 view = _camera.GetViewMatrix();
            projection = glm::perspective(glm::radians(45.0f), (GLfloat)width() / (GLfloat)height(), 0.1f, 100.0f);

            _program.SetUniform("view", view);
            _program.SetUniform("projection", projection);

            _program.SetUniform("viewPos", _camera.Position);

            _program.SetUniform("material.diffuse", 0);
            _program.SetUniform("material.specular", 1);
            _program.SetUniform("material.shininess", 32.0f);

            // TODO : UBO
            //const auto& testLight = lights[0];

            /*
               Here we set all the uniforms for the 5/6 types of lights we have. We have to set them manually and index
               the proper PointLight struct in the array to set each uniform variable. This can be done more code-friendly
               by defining light types as classes and set their values in there, or by using a more efficient uniform approach
               by using 'Uniform buffer objects', but that is something we'll discuss in the 'Advanced GLSL' tutorial.
            */
            // TODO ; USE UBO!!
            // directional light

            int pointLightIndex = 0;
            for (int i = 0; i < lights.size(); i++)
            {
                const auto& light = lights[i];

                if (light.type == LightType::Directional)
                {
                    _program.SetUniform("dirLight.direction", light.direction);
                    _program.SetUniform("dirLight.ambient", light.ambient);
                    _program.SetUniform("dirLight.diffuse", light.diffuse);
                    _program.SetUniform("dirLight.specular", light.specular);
                }
                if (light.type == LightType::Point)
                {
                    _program.SetUniform(std::format("pointLights[{}].position", pointLightIndex), light.position);
                    _program.SetUniform(std::format("pointLights[{}].ambient", pointLightIndex), light.ambient);
                    _program.SetUniform(std::format("pointLights[{}].diffuse", pointLightIndex), light.diffuse);
                    _program.SetUniform(std::format("pointLights[{}].specular", pointLightIndex), light.specular);
                    _program.SetUniform(std::format("pointLights[{}].constant", pointLightIndex), light.constant);
                    _program.SetUniform(std::format("pointLights[{}].linear", pointLightIndex), light.linear);
                    _program.SetUniform(std::format("pointLights[{}].quadratic", pointLightIndex), light.quadratic);
                    pointLightIndex++;
                }
                else if (light.type == LightType::Spot)
                {
                    _program.SetUniform("spotLight.position", light.position);
                    _program.SetUniform("spotLight.direction", light.direction);
                    _program.SetUniform("spotLight.ambient", light.ambient);
                    _program.SetUniform("spotLight.diffuse", light.diffuse);
                    _program.SetUniform("spotLight.specular", light.specular);
                    _program.SetUniform("spotLight.constant", light.constant);
                    _program.SetUniform("spotLight.linear", light.linear);
                    _program.SetUniform("spotLight.quadratic", light.quadratic);
                    _program.SetUniform("spotLight.cutOff", light.cutOff);
                    _program.SetUniform("spotLight.outerCutOff", light.outerCutOff);
                }
            }


            // point lights 
            //for (int i = 0; i < pointLights.size(); i++)
            //{
            //    const auto& light = pointLights[i];

            //    _program.SetUniform(std::format("pointLights[{}].position", i), light.position);
            //    _program.SetUniform(std::format("pointLights[{}].ambient", i), light.ambient);
            //    _program.SetUniform(std::format("pointLights[{}].diffuse", i), light.diffuse);
            //    _program.SetUniform(std::format("pointLights[{}].specular", i), light.specular);
            //    _program.SetUniform(std::format("pointLights[{}].constant", i), light.constant);
            //    _program.SetUniform(std::format("pointLights[{}].linear", i), light.linear);
            //    _program.SetUniform(std::format("pointLights[{}].quadratic", i), light.quadratic);
            //}

            // spotLight


            _cubeDiffuseTexture.Bind();
            _cubeSpecularTexture.Bind(1);
            _cubeVao.Bind();

            glm::mat4 model = glm::mat4{ 1.0f };

            for (GLuint i = 0; i < 10; i++)
            {
                model = glm::translate(glm::mat4{ 1.0f }, cubePositions[i]);
                float angle = 20.0f * i;
                if (i % 3 == 0)  // every 3rd iteration (including the first) we set the angle using GLFW's time function.
                    angle = glfwGetTime() * 25.0f;
                model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
                _program.SetUniform("model", model);

                Render::OpenGL::Commands::DrawVertices(Render::PrimitiveType::Triangles, 36);
            }

            // model
            {
                model = glm::scale(glm::mat4{ 1.0f }, glm::vec3(0.1f));
                _program.SetUniform("model", model);
                _model.Render();
            }

            // floor
            {
                _planeVao.Bind();
                _metalAlbedo.Bind();
                glBindTextureUnit(1, 0);

                model = glm::mat4{ 1.0f };
                _program.SetUniform("model", model);
                Render::OpenGL::Commands::DrawVertices(Render::PrimitiveType::Triangles, 6);
            }

            _lampProgram.Bind();

            _lampProgram.SetUniform("view", view);
            _lampProgram.SetUniform("projection", projection);

            _lightSourceVao.Bind();
            for (unsigned int i = 0; i < lights.size(); i++)
            {
                const auto& light = lights[i];

                if (light.type == LightType::Point)
                {
                    model = glm::mat4(1.0f);
                    model = glm::translate(model, light.position);
                    model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
                    _lampProgram.SetUniform("model", model);
                    _lampProgram.SetUniform("lightColor", light.diffuse);
                    Render::OpenGL::Commands::DrawVertices(Render::PrimitiveType::Triangles, 36);
                }
            }

        }

        bool keys[1024];
        bool cull_face{ true };

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

            if (key == GLFW_KEY_F2 && action == GLFW_PRESS)
            {
                cull_face = !cull_face;

                if (cull_face)
                    Render::OpenGL::Pipeline::Disable(Render::PipelineFeature::CullFace);
                else
                    Render::OpenGL::Pipeline::Enable(Render::PipelineFeature::CullFace);
            }

            if (key == GLFW_KEY_F1 && action == GLFW_PRESS)
            {
                _lockCursor = !_lockCursor;
                glfwSetInputMode(WindowHandle(), GLFW_CURSOR, _lockCursor ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
            }

            if (key == GLFW_KEY_1 && action == GLFW_PRESS)
            {
                _selectedPipeline = 0;
            }
            if (key == GLFW_KEY_2 && action == GLFW_PRESS)
            {
                _selectedPipeline = 1;
            }
            if (key == GLFW_KEY_3 && action == GLFW_PRESS)
            {
                _selectedPipeline = 2;
            }
            if (key == GLFW_KEY_4 && action == GLFW_PRESS)
            {
                _selectedPipeline = 3;
            }
            if (key == GLFW_KEY_5 && action == GLFW_PRESS)
            {
                _selectedPipeline = 4;
            }

            if (action == GLFW_PRESS)
                keys[key] = true;
            else if (action == GLFW_RELEASE)
                keys[key] = false;
        }

        void onMouseHandle(double xPos, double yPos) override
        {
            if (!_lockCursor)
            {
                firstMouse = true;
                return;
            }


            if (firstMouse)
            {
                lastX = xPos;
                lastY = yPos;
                firstMouse = false;
            }

            GLfloat xoffset = xPos - lastX;
            GLfloat yoffset = lastY - yPos; // ќбратный пор€док вычитани€ потому что оконные Y-координаты возрастают с верху вниз 

            lastX = xPos;
            lastY = yPos;

            _camera.ProcessMouseMovement(xoffset, yoffset);
        }

    private:
        Render::Model _model;

        Render::OpenGL::Texture2D _cubeDiffuseTexture;
        Render::OpenGL::Texture2D _cubeSpecularTexture;
        Render::OpenGL::Texture2D _metalAlbedo;

        Render::OpenGL::VertexArray _cubeVao;
        Render::OpenGL::VertexArray _planeVao;
        Render::OpenGL::VertexArray _lightSourceVao;
        Render::OpenGL::VertexArray _quadVao;
        Render::OpenGL::VertexArray _quad2Vao;

        Render::OpenGL::ShaderProgram _program;
        Render::OpenGL::ShaderProgram _lampProgram;
        Render::OpenGL::ShaderProgram _screenProgram;
        Render::OpenGL::ShaderProgram _colorInverseProgram;
        Render::OpenGL::ShaderProgram _grayScaleProgram;
        Render::OpenGL::ShaderProgram _kernelEffectProgram;
        Render::OpenGL::ShaderProgram _blurEffectProgram;
        Render::OpenGL::ShaderProgram _tonemapPipeline;

        GLfloat lastX, lastY;
        bool firstMouse{ true };

        bool _isLineMode{};
        bool _lockCursor{ true };
    };
}