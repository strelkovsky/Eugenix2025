#pragma once

#include <imgui/imgui.h>

// Sandbox headers
#include "Render/OpenGL/Buffer.h"
#include "Render/OpenGL/Commands.h"
#include "Render/OpenGL/Pipeline.h"
#include "Render/OpenGL/Sampler.h"
#include "Render/OpenGL/Texture2D.h"
#include "Render/OpenGL/VertexArray.h"
#include "Render/Types.h"

#include "LearnOpenGL-Shared.h"
#include "LearnOpenGLApp-Base.h"

namespace Eugenix
{
    std::vector<glm::vec3> vegetation =
    {
        glm::vec3(-1.5f, 0.0f, -0.48f),
        glm::vec3(1.5f, 0.0f, 0.51f),
        glm::vec3(0.0f, 0.0f, 0.7f),
        glm::vec3(-0.3f, 0.0f, -2.3f),
        glm::vec3(0.5f, 0.0f, -0.6f)
    };

    int selectedLightIndex;

    // TODO : lights UBO from Eugenix. UBO Test. ShaderEnv stuff

    class LearnOpenGLColorBlendingApp final : public LearnOpenGLAppBase
    {
    protected:
        bool onInit() override
        {
            LearnOpenGLAppBase::onInit();

            _program = MakeProgramFromFiles("shaders/SimpleVertex.vert", "shaders/SimplePhong.frag");
            _lampProgram = MakeProgramFromFiles("shaders/SimpleVertex.vert", "shaders/SimpleUnlit.frag");

            const std::vector<Render::Vertex::PosNormalUV> grassVertices =
            {
                {{0.0f,  0.5f,  0.0f}, {0.0f,  0.0f,  1.0f}, { 0.0f,  0.0f }},
                {{0.0f, -0.5f,  0.0f}, {0.0f,  0.0f,  1.0f}, {0.0f,  1.0f}},
                {{1.0f, -0.5f,  0.0f}, {0.0f,  0.0f,  1.0f}, {1.0f,  1.0f}},
                                        
                {{0.0f,  0.5f,  0.0f}, {0.0f,  0.0f,  1.0f}, {0.0f,  0.0f}},
                {{1.0f, -0.5f,  0.0f}, {0.0f,  0.0f,  1.0f}, {1.0f,  1.0f}},
                {{1.0f,  0.5f,  0.0f}, {0.0f,  0.0f,  1.0f}, {1.0f,  0.0f}}
            };

            Render::OpenGL::Buffer grassVbo;
            grassVbo.Create();
            grassVbo.Storage(Core::MakeData(std::span{ grassVertices }));

            _grassVao.Create();
            _grassVao.AttachVertices(grassVbo, sizeof(Render::Vertex::PosNormalUV));
            _grassVao.Attribute(position_attribute);
            _grassVao.Attribute(normal_attribute);
            _grassVao.Attribute(uv_attribute);

            auto img = _imageLoader.Load("Textures/container2.png");
            _cubeDiffuseTexture.Create();
            _cubeDiffuseTexture.Upload(img);

            img = _imageLoader.Load("Textures/container2_specular.png");
            _cubeSpecularTexture.Create();
            _cubeSpecularTexture.Upload(img, {.colorSpace = Render::TextureColorSpace::Linear});

            img = _imageLoader.Load("Textures/metal.png");
            _metalAlbedo.Create();
            _metalAlbedo.Upload(img);

            img = _imageLoader.Load("Textures/blending_transparent_window.png");
            _transparentTexture.Create();
            _transparentTexture.Upload(img);
            
            _defaultSampler.Create();
            _defaultSampler.Parameter(Render::TextureParam::WrapS, Render::TextureWrapping::Repeat);
            _defaultSampler.Parameter(Render::TextureParam::WrapT, Render::TextureWrapping::Repeat);
            _defaultSampler.Parameter(Render::TextureParam::MinFilter, Render::TextureFilter::Linear);
            _defaultSampler.Parameter(Render::TextureParam::MagFilter, Render::TextureFilter::Linear);

            _alphaSampler.Create();
            _alphaSampler.Parameter(Render::TextureParam::WrapS, Render::TextureWrapping::ClampToEdge);
            _alphaSampler.Parameter(Render::TextureParam::WrapT, Render::TextureWrapping::ClampToEdge);
            _alphaSampler.Parameter(Render::TextureParam::MinFilter, Render::TextureFilter::Linear);
            _alphaSampler.Parameter(Render::TextureParam::MagFilter, Render::TextureFilter::Linear);

            Render::OpenGL::Commands::Clear(0.1f, 0.1f, 0.1f);

            Render::OpenGL::Pipeline::Enable(Render::PipelineFeature::DepthTest);
            Render::OpenGL::Pipeline::Enable(Render::PipelineFeature::Blend);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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

        void onDebugUI() override
        {
            ImGui::Begin("Light Control");

            int lightCount = std::max(1, (int)lights.size());
            ImGui::SliderInt("Select Light", &selectedLightIndex, 0, lightCount - 1);

            static const char* light_types[] = { "Directional", "Point", "Spot" };

            auto& selectedLight = lights[selectedLightIndex];

            int type = static_cast<int>(selectedLight.type);
            if (ImGui::Combo("Light Type", &type, light_types, IM_ARRAYSIZE(light_types)))
                selectedLight.type = static_cast<LightType>(type);

            if (selectedLight.type == LightType::Directional || selectedLight.type == LightType::Spot)
            {
                ImGui::SliderFloat3("Direction", glm::value_ptr(selectedLight.direction), -1.0f, 1.0f);
                selectedLight.direction = glm::normalize(selectedLight.direction);
            }

            if (selectedLight.type == LightType::Point || selectedLight.type == LightType::Spot)
            {
                ImGui::SliderFloat3("Position", glm::value_ptr(selectedLight.position), -10.0f, 10.0f);
            }

            ImGui::ColorEdit3("Ambient", glm::value_ptr(selectedLight.ambient));
            ImGui::ColorEdit3("Diffuse", glm::value_ptr(selectedLight.diffuse));
            ImGui::ColorEdit3("Specular", glm::value_ptr(selectedLight.specular));

            if (selectedLight.type == LightType::Point)
            {
                ImGui::SliderFloat("Constant", &selectedLight.constant, 0.5f, 2.0f);
                ImGui::SliderFloat("Linear", &selectedLight.linear, 0.001f, 0.2f);
                ImGui::SliderFloat("Quadratic", &selectedLight.quadratic, 0.0001f, 0.1f);
            }

            if (selectedLight.type == LightType::Spot)
            {
                ImGui::SliderFloat("CutOff", &selectedLight.cutOff, 0.0f, glm::pi<float>());
            }

            ImGui::End();
        }

        void onRender() override
        {
            Render::OpenGL::Commands::Viewport(0, 0, width(), height());
            Render::OpenGL::Commands::Clear(Render::ClearFlags::Color | Render::ClearFlags::Depth);

            glm::mat4 projection{ 1.0f };

            // rotate camera around (0, 0, 0)
            //GLfloat radius = 10.0f;
            //GLfloat camX = sin(glfwGetTime()) * radius;
            //GLfloat camZ = cos(glfwGetTime()) * radius;
            //view = glm::lookAt(glm::vec3(camX, 0.0, camZ), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));

            Render::OpenGL::Pipeline::Disable(Render::PipelineFeature::Blend);

            _program.Bind();

            glm::mat4 view = _camera.GetViewMatrix();
            projection = glm::perspective(glm::radians(45.0f), (GLfloat)width() / (GLfloat)height(), 0.1f, 100.0f);

            _program.SetUniform("view", view);
            _program.SetUniform("projection", projection);

            _program.SetUniform("viewPos", _camera.Position);

            // TODO : UBO!!
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

            //    _pipeline.SetUniform(std::format("pointLights[{}].position", i), light.position);
            //    _pipeline.SetUniform(std::format("pointLights[{}].ambient", i), light.ambient);
            //    _pipeline.SetUniform(std::format("pointLights[{}].diffuse", i), light.diffuse);
            //    _pipeline.SetUniform(std::format("pointLights[{}].specular", i), light.specular);
            //    _pipeline.SetUniform(std::format("pointLights[{}].constant", i), light.constant);
            //    _pipeline.SetUniform(std::format("pointLights[{}].linear", i), light.linear);
            //    _pipeline.SetUniform(std::format("pointLights[{}].quadratic", i), light.quadratic);
            //}

            // spotLight


            _cubeDiffuseTexture.Bind();
            _cubeSpecularTexture.Bind(1);
            _defaultSampler.Bind(0);
            _cubeVao.Bind();

            _program.SetUniform("material.alphaMode", 2);

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
                glBindTextureUnit(1, 0); // ??

                model = glm::mat4{ 1.0f };
                _program.SetUniform("model", model);
                Render::OpenGL::Commands::DrawVertices(Render::PrimitiveType::Triangles, 6);
            }

            Render::OpenGL::Pipeline::Enable(Render::PipelineFeature::Blend);

            // grass
            {
                _alphaSampler.Bind(0);

                _program.SetUniform("material.alphaMode", 2);

                _grassVao.Bind();
                _transparentTexture.Bind();

                // sort the transparent windows before rendering
                // ---------------------------------------------
                std::map<float, glm::vec3> sorted;
                for (unsigned int i = 0; i < vegetation.size(); i++)
                {
                    float distance = glm::length(_camera.Position - vegetation[i]);
                    sorted[distance] = vegetation[i];
                }

                for (std::map<float, glm::vec3>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it)
                {
                    model = glm::mat4(1.0f);
                    model = glm::translate(model, it->second);
                    _program.SetUniform("model", model);
                    Render::OpenGL::Commands::DrawVertices(Render::PrimitiveType::Triangles, 6);
                }
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
            GLfloat yoffset = lastY - yPos; // Обратный порядок вычитания потому что оконные Y-координаты возрастают с верху вниз 

            lastX = xPos;
            lastY = yPos;

            _camera.ProcessMouseMovement(xoffset, yoffset);
        }

    private:
        Render::Model _model;

        Render::OpenGL::Texture2D _cubeDiffuseTexture;
        Render::OpenGL::Texture2D _cubeSpecularTexture;
        Render::OpenGL::Texture2D _metalAlbedo;
        Render::OpenGL::Texture2D _transparentTexture;

        Render::OpenGL::Sampler _defaultSampler;
        Render::OpenGL::Sampler _alphaSampler;

        Render::OpenGL::VertexArray _grassVao;

        Render::OpenGL::ShaderProgram _program;
        Render::OpenGL::ShaderProgram _lampProgram;

        GLfloat lastX, lastY;
        bool firstMouse{ true };

        bool _isLineMode{};
        bool _lockCursor{ true };
    };
}