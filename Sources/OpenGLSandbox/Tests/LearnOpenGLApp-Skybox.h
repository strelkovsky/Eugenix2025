#pragma once

#include "LearnOpenGL-Shared.h"
#include "LearnOpenGLApp-Base.h"

namespace Eugenix
{
    class LearnOpenGLSkyboxApp final : public LearnOpenGLAppBase
    {
    protected:

        // TODO :  Use cubemap class
        unsigned int loadCubemap(std::vector<std::string> faces)
        {
            unsigned int textureID;
            glGenTextures(1, &textureID);
            glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

            int width, height, nrChannels;
            for (unsigned int i = 0; i < faces.size(); i++)
            {
                unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
                if (data)
                {
                    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                        0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
                    );
                    stbi_image_free(data);
                }
                else
                {
                    std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
                    stbi_image_free(data);
                }
            }

            // TODO : Use sampler
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

            return textureID;
        }

        unsigned int cubemapTexture;

        std::vector<std::string> faces =
        {
                "textures/skybox/right.jpg",
                "textures/skybox/left.jpg",
                "textures/skybox/top.jpg",
                "textures/skybox/bottom.jpg",
                "textures/skybox/front.jpg",
                "textures/skybox/back.jpg"
        };

        bool onInit() override
        {
            LearnOpenGLAppBase::onInit();

            cubemapTexture = loadCubemap(faces);

            _skyboxProgram = MakeProgramFromFiles("shaders/SimpleSkybox.vert", "shaders/SimpleSkybox.frag");

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

            const std::vector<Render::Vertex::Pos> skyboxVertices =
            {
                // positions          
                {{-1.0f,  1.0f, -1.0f}},
                {{-1.0f, -1.0f, -1.0f}},
                {{ 1.0f, -1.0f, -1.0f}},
                {{ 1.0f, -1.0f, -1.0f}},
                {{ 1.0f,  1.0f, -1.0f}},
                {{-1.0f,  1.0f, -1.0f}},

                {{-1.0f, -1.0f,  1.0f}},
                {{-1.0f, -1.0f, -1.0f}},
                {{-1.0f,  1.0f, -1.0f}},
                {{-1.0f,  1.0f, -1.0f}},
                {{-1.0f,  1.0f,  1.0f}},
                {{-1.0f, -1.0f,  1.0f}},

                {{ 1.0f, -1.0f, -1.0f}},
                {{ 1.0f, -1.0f,  1.0f}},
                {{ 1.0f,  1.0f,  1.0f}},
                {{ 1.0f,  1.0f,  1.0f}},
                {{ 1.0f,  1.0f, -1.0f}},
                {{ 1.0f, -1.0f, -1.0f}},

                {{-1.0f, -1.0f,  1.0f}},
                {{-1.0f,  1.0f,  1.0f}},
                {{ 1.0f,  1.0f,  1.0f}},
                {{ 1.0f,  1.0f,  1.0f}},
                {{ 1.0f, -1.0f,  1.0f}},
                {{-1.0f, -1.0f,  1.0f}},

                {{-1.0f,  1.0f, -1.0f}},
                {{ 1.0f,  1.0f, -1.0f}},
                {{ 1.0f,  1.0f,  1.0f}},
                {{ 1.0f,  1.0f,  1.0f}},
                {{-1.0f,  1.0f,  1.0f}},
                {{-1.0f,  1.0f, -1.0f}},

                {{-1.0f, -1.0f, -1.0f}},
                {{-1.0f, -1.0f,  1.0f}},
                {{ 1.0f, -1.0f, -1.0f}},
                {{ 1.0f, -1.0f, -1.0f}},
                {{-1.0f, -1.0f,  1.0f}},
                {{ 1.0f, -1.0f,  1.0f}}
            };

            // screen quad VAO
            Render::OpenGL::Buffer quadVbo;
            quadVbo.Create();
            quadVbo.Storage(Core::MakeData(std::span{ quadVertices }));

            _quadVao.Create();
            _quadVao.AttachVertices(quadVbo, sizeof(Render::Vertex::Sprite));
            _quadVao.Attribute({ 0, 2, Render::DataType::Float, false, 0 });
            _quadVao.Attribute({ 1, 2, Render::DataType::Float, false, offsetof(Render::Vertex::Sprite, uv) });

            // Skybox
            Render::OpenGL::Buffer skyboxVbo;
            skyboxVbo.Create();
            skyboxVbo.Storage(Core::MakeData(std::span{ skyboxVertices }));

            _skyboxVao.Create();
            _skyboxVao.AttachVertices(skyboxVbo, sizeof(Render::Vertex::Pos));
            _skyboxVao.Attribute({ 0, 3, Render::DataType::Float, false, 0 });

            Render::OpenGL::Commands::Clear(0.1f, 0.1f, 0.1f);

            Render::OpenGL::Pipeline::Enable(Render::PipelineFeature::DepthTest);

            glFrontFace(GL_CCW);        // лицевые треугольники Ц против часовой стрелки
            glCullFace(GL_BACK);        // отбрасывать задние

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
        }

        void onResize() override
        {
        }

        void onRender() override
        {
            Render::OpenGL::Pipeline::Enable(Render::PipelineFeature::DepthTest);
            Render::OpenGL::Commands::Viewport(0, 0, width(), height());
            Render::OpenGL::Commands::Clear(Render::ClearFlags::Color | Render::ClearFlags::Depth);
            drawScene();

            //_skyboxProgram.SetUniform("skybox", 0);

            // draw skybox as last
            glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
            _skyboxProgram.Bind();
            glm::mat4 view = glm::mat4(glm::mat3(_camera.GetViewMatrix())); // remove translation from the view matrix
            auto projection = glm::perspective(glm::radians(45.0f), (GLfloat)width() / (GLfloat)height(), 0.1f, 100.0f);
            _skyboxProgram.SetUniform("view", view);
            _skyboxProgram.SetUniform("projection", projection);
            // skybox cube
            _skyboxVao.Bind();
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            glBindVertexArray(0);
            glDepthFunc(GL_LESS); // set depth function back to default
        }

        void drawScene()
        {
            glm::mat4 projection{ 1.0f };

            // rotate camera around (0, 0, 0)
            //GLfloat radius = 10.0f;
            //GLfloat camX = sin(glfwGetTime()) * radius;
            //GLfloat camZ = cos(glfwGetTime()) * radius;
            //view = glm::lookAt(glm::vec3(camX, 0.0, camZ), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));

            _defaultShader.Bind();

            glm::mat4 view = _camera.GetViewMatrix();
            projection = glm::perspective(glm::radians(45.0f), (GLfloat)width() / (GLfloat)height(), 0.1f, 100.0f);

            _defaultShader.SetUniform("view", view);
            _defaultShader.SetUniform("projection", projection);

            _defaultShader.SetUniform("viewPos", _camera.Position);

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
                glBindTextureUnit(1, 0);

                model = glm::mat4{ 1.0f };
                _defaultShader.SetUniform("model", model);
                Render::OpenGL::Commands::DrawVertices(Render::PrimitiveType::Triangles, 6);
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

        bool cull_face{ true };

        void onKeyHandle(int key, int code, int action, int mode) override
        {
            LearnOpenGLAppBase::onKeyHandle(key, code, action, mode);
        }

        void onMouseHandle(double xPos, double yPos) override
        {
            LearnOpenGLAppBase::onMouseHandle(xPos, yPos);
        }

    private:
        Render::OpenGL::VertexArray _quadVao;
        Render::OpenGL::VertexArray _skyboxVao;

        Render::OpenGL::ShaderProgram _skyboxProgram;
    };
}