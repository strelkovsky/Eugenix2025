#pragma once

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

    // TODO : lights UBO from Eugenix. UBO Test. ShaderEnv stuff

    class LearnOpenGLColorBlendingApp final : public LearnOpenGLAppBase
    {
    protected:
        bool onInit() override
        {
            LearnOpenGLAppBase::onInit();

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

            const auto img = _imageLoader.Load("Textures/blending_transparent_window.png");
            _transparentTexture.Create();
            _transparentTexture.Upload(img);
           
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

            _lastX = (float)width() / 2.0f, _lastY = (float)height() / 2.0f;

            _model = _modelLoader.Load("Models/nanosuit/nanosuit.obj");

            return true;
        }

        void onUpdate(float deltaTime) override
        {
            proceedCamera(deltaTime);
            proceedLights(deltaTime);
        }

        void onDebugUI() override
        {
            ImGui::Begin("Light Control");

            int lightCount = std::max(1, (int)lights.size());
            ImGui::SliderInt("Select Light", &_selectedLightIndex, 0, lightCount - 1);

            static const char* light_types[] = { "Directional", "Point", "Spot" };

            auto& selectedLight = lights[_selectedLightIndex];

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

            _defaultShader.Bind();

            glm::mat4 view = _camera.GetViewMatrix();
            projection = glm::perspective(glm::radians(45.0f), (GLfloat)width() / (GLfloat)height(), 0.1f, 100.0f);

            _defaultShader.SetUniform("view", view);
            _defaultShader.SetUniform("projection", projection);

            _defaultShader.SetUniform("viewPos", _camera.Position);

            // TODO : UBO!!
            _defaultShader.SetUniform("material.diffuse", 0);
            _defaultShader.SetUniform("material.specular", 1);
            _defaultShader.SetUniform("material.shininess", 32.0f);

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
                    _defaultShader.SetUniform("dirLight.direction", light.direction);
                    _defaultShader.SetUniform("dirLight.ambient", light.ambient);
                    _defaultShader.SetUniform("dirLight.diffuse", light.diffuse);
                    _defaultShader.SetUniform("dirLight.specular", light.specular);
                }
                if (light.type == LightType::Point)
                {
                    _defaultShader.SetUniform(std::format("pointLights[{}].position", pointLightIndex), light.position);
                    _defaultShader.SetUniform(std::format("pointLights[{}].ambient", pointLightIndex), light.ambient);
                    _defaultShader.SetUniform(std::format("pointLights[{}].diffuse", pointLightIndex), light.diffuse);
                    _defaultShader.SetUniform(std::format("pointLights[{}].specular", pointLightIndex), light.specular);
                    _defaultShader.SetUniform(std::format("pointLights[{}].constant", pointLightIndex), light.constant);
                    _defaultShader.SetUniform(std::format("pointLights[{}].linear", pointLightIndex), light.linear);
                    _defaultShader.SetUniform(std::format("pointLights[{}].quadratic", pointLightIndex), light.quadratic);
                    pointLightIndex++;
                }
                else if (light.type == LightType::Spot)
                {
                    _defaultShader.SetUniform("spotLight.position", light.position);
                    _defaultShader.SetUniform("spotLight.direction", light.direction);
                    _defaultShader.SetUniform("spotLight.ambient", light.ambient);
                    _defaultShader.SetUniform("spotLight.diffuse", light.diffuse);
                    _defaultShader.SetUniform("spotLight.specular", light.specular);
                    _defaultShader.SetUniform("spotLight.constant", light.constant);
                    _defaultShader.SetUniform("spotLight.linear", light.linear);
                    _defaultShader.SetUniform("spotLight.quadratic", light.quadratic);
                    _defaultShader.SetUniform("spotLight.cutOff", light.cutOff);
                    _defaultShader.SetUniform("spotLight.outerCutOff", light.outerCutOff);
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

            _defaultShader.SetUniform("material.alphaMode", 2);

            glm::mat4 model = glm::mat4{ 1.0f };

            for (GLuint i = 0; i < 10; i++)
            {
                model = glm::translate(glm::mat4{ 1.0f }, cubePositions[i]);
                float angle = 20.0f * i;
                if (i % 3 == 0)  // every 3rd iteration (including the first) we set the angle using GLFW's time function.
                    angle = glfwGetTime() * 25.0f;
                model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
                _defaultShader.SetUniform("model", model);

                Render::OpenGL::Commands::DrawVertices(Render::PrimitiveType::Triangles, 36);
            }

            // model
            {
                model = glm::scale(glm::mat4{ 1.0f }, glm::vec3(0.1f));
                _defaultShader.SetUniform("model", model);
                _model.Render();
            }

            // floor
            {
                _planeVao.Bind();
                _metalAlbedo.Bind();
                glBindTextureUnit(1, 0); // ??

                model = glm::mat4{ 1.0f };
                _defaultShader.SetUniform("model", model);
                Render::OpenGL::Commands::DrawVertices(Render::PrimitiveType::Triangles, 6);
            }

            Render::OpenGL::Pipeline::Enable(Render::PipelineFeature::Blend);

            // grass
            {
                _alphaSampler.Bind(0);

                _defaultShader.SetUniform("material.alphaMode", 2);

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
                    _defaultShader.SetUniform("model", model);
                    Render::OpenGL::Commands::DrawVertices(Render::PrimitiveType::Triangles, 6);
                }
            }

            _lampShader.Bind();

            _lampShader.SetUniform("view", view);
            _lampShader.SetUniform("projection", projection);

            _lightSourceVao.Bind();
            for (unsigned int i = 0; i < lights.size(); i++)
            {
                const auto& light = lights[i];

                if (light.type == LightType::Point)
                {
                    model = glm::mat4(1.0f);
                    model = glm::translate(model, light.position);
                    model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
                    _lampShader.SetUniform("model", model);
                    _lampShader.SetUniform("lightColor", light.diffuse);
                    Render::OpenGL::Commands::DrawVertices(Render::PrimitiveType::Triangles, 36);
                }
            }
        }

        void onKeyHandle(int key, int code, int action, int mode) override
        {
            LearnOpenGLAppBase::onKeyHandle(key, code, action, mode);
        }

        void onMouseHandle(double xPos, double yPos) override
        {
            LearnOpenGLAppBase::onMouseHandle(xPos, yPos);
        }

    private:
        Render::OpenGL::Texture2D _transparentTexture;
        Render::OpenGL::Sampler _alphaSampler;

        Render::OpenGL::VertexArray _grassVao;
    };
}