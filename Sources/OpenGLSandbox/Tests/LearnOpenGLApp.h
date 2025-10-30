#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui/imgui.h>

// Sandbox headers
#include "App/SandboxApp.h"
#include "Assets/ImageLoader.h"
#include "Assets/ObjModelLoader.h"
#include "Render/OpenGL/Buffer.h"
#include "Render/OpenGL/Commands.h"
#include "Render/OpenGL/Pipeline.h"
#include "Render/OpenGL/ShaderStage.h"
#include "Render/OpenGL/Texture2D.h"
#include "Render/OpenGL/VertexArray.h"
#include "Render/Types.h"

namespace Eugenix
{
    struct Vertex
    {
        glm::vec3 pos;
        glm::vec3 normal;
        glm::vec2 uv;

        // TODO : return span of attributes
    };

    glm::vec3 cubePositions[] = {
          glm::vec3(0.0f,  0.0f,  0.0f),
          glm::vec3(2.0f,  5.0f, -15.0f),
          glm::vec3(-1.5f, -2.2f, -2.5f),
          glm::vec3(-3.8f, -2.0f, -12.3f),
          glm::vec3(2.4f, -0.4f, -3.5f),
          glm::vec3(-1.7f,  3.0f, -7.5f),
          glm::vec3(1.3f, -2.0f, -2.5f),
          glm::vec3(1.5f,  2.0f, -2.5f),
          glm::vec3(1.5f,  0.2f, -1.5f),
          glm::vec3(-1.3f,  1.0f, -1.5f)
    };



    // Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
    enum Camera_Movement {
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT
    };

    // Default camera values
    const GLfloat YAW = -90.0f;
    const GLfloat PITCH = 0.0f;
    const GLfloat SPEED = 3.0f;
    const GLfloat SENSITIVTY = 0.25f;
    const GLfloat ZOOM = 45.0f;


    // An abstract camera class that processes input and calculates the corresponding Eular Angles, Vectors and Matrices for use in OpenGL
    class Camera2
    {
    public:
        // Camera Attributes
        glm::vec3 Position;
        glm::vec3 Front;
        glm::vec3 Up;
        glm::vec3 Right;
        glm::vec3 WorldUp;
        // Eular Angles
        GLfloat Yaw;
        GLfloat Pitch;
        // Camera options
        GLfloat MovementSpeed;
        GLfloat MouseSensitivity;
        GLfloat Zoom;

        // Constructor with vectors
        Camera2(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), GLfloat yaw = YAW, GLfloat pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVTY), Zoom(ZOOM)
        {
            this->Position = position;
            this->WorldUp = up;
            this->Yaw = yaw;
            this->Pitch = pitch;
            this->updateCameraVectors();
        }
        // Constructor with scalar values
        Camera2(GLfloat posX, GLfloat posY, GLfloat posZ, GLfloat upX, GLfloat upY, GLfloat upZ, GLfloat yaw, GLfloat pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVTY), Zoom(ZOOM)
        {
            this->Position = glm::vec3(posX, posY, posZ);
            this->WorldUp = glm::vec3(upX, upY, upZ);
            this->Yaw = yaw;
            this->Pitch = pitch;
            this->updateCameraVectors();
        }

        // Returns the view matrix calculated using Eular Angles and the LookAt Matrix
        glm::mat4 GetViewMatrix()
        {
            return glm::lookAt(this->Position, this->Position + this->Front, this->Up);
        }

        // Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
        void ProcessKeyboard(Camera_Movement direction, GLfloat deltaTime)
        {
            GLfloat velocity = this->MovementSpeed * deltaTime;
            if (direction == FORWARD)
                this->Position += this->Front * velocity;
            if (direction == BACKWARD)
                this->Position -= this->Front * velocity;
            if (direction == LEFT)
                this->Position -= this->Right * velocity;
            if (direction == RIGHT)
                this->Position += this->Right * velocity;
        }

        // Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
        void ProcessMouseMovement(GLfloat xoffset, GLfloat yoffset, GLboolean constrainPitch = true)
        {
            xoffset *= this->MouseSensitivity;
            yoffset *= this->MouseSensitivity;

            this->Yaw += xoffset;
            this->Pitch += yoffset;

            // Make sure that when pitch is out of bounds, screen doesn't get flipped
            if (constrainPitch)
            {
                if (this->Pitch > 89.0f)
                    this->Pitch = 89.0f;
                if (this->Pitch < -89.0f)
                    this->Pitch = -89.0f;
            }

            // Update Front, Right and Up Vectors using the updated Eular angles
            this->updateCameraVectors();
        }

        // Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
        void ProcessMouseScroll(GLfloat yoffset)
        {
            if (this->Zoom >= 1.0f && this->Zoom <= 45.0f)
                this->Zoom -= yoffset;
            if (this->Zoom <= 1.0f)
                this->Zoom = 1.0f;
            if (this->Zoom >= 45.0f)
                this->Zoom = 45.0f;
        }

    private:
        // Calculates the front vector from the Camera's (updated) Eular Angles
        void updateCameraVectors()
        {
            // Calculate the new Front vector
            glm::vec3 front;
            front.x = cos(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch));
            front.y = sin(glm::radians(this->Pitch));
            front.z = sin(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch));
            this->Front = glm::normalize(front);
            // Also re-calculate the Right and Up vector
            this->Right = glm::normalize(glm::cross(this->Front, this->WorldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
            this->Up = glm::normalize(glm::cross(this->Right, this->Front));
        }
    };

    Camera2 camera{ glm::vec3(0.0f, 0.0f, 4.0f) };

    enum class LightType : int
    {
        Directional = 0,
        Point = 1,
        Spot = 2
    };

    struct Light
    {
        LightType type;

        // directional
        glm::vec3 direction;
        // posint & spot
        glm::vec3 position;

        glm::vec3 ambient;
        glm::vec3 diffuse;
        glm::vec3 specular;

        // point 
        float constant;
        float linear;
        float quadratic;

        // Spot
        float cutOff;
        float outerCutOff;
    };

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
            .direction = camera.Front,
            .position = camera.Position,
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

    int selectedLightIndex;

    // TODO : lights UBO from Eugenix. UBO Test. ShaderEnv stuff

    class LearnOpenGLApp final : public SandboxApp
    {
    protected:
        bool onInit() override
        {
            _pipeline = MakePipelineFromFiles("shaders/SimpleVertex.vert", "shaders/SimplePhong.frag");
            _lampPipeline = MakePipelineFromFiles("shaders/SimpleVertex.vert", "shaders/SimpleUnlit.frag");

            // Set up vertex data (and buffer(s)) and attribute pointers
            const std::vector<Vertex> vertices =
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

            const std::vector<Vertex> planeVertices =
            {
                {{ 5.0f, -0.5f,  5.0f},  {0.0f, 1.0f, 0.0f}, {2.0f, 0.0f}},
                {{-5.0f, -0.5f,  5.0f},  {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
                {{-5.0f, -0.5f, -5.0f},  {0.0f, 1.0f, 0.0f}, {0.0f, 2.0f}},

                {{ 5.0f, -0.5f,  5.0f},  {0.0f, 1.0f, 0.0f}, {2.0f, 0.0f}},
                {{-5.0f, -0.5f, -5.0f},  {0.0f, 1.0f, 0.0f}, {0.0f, 2.0f}},
                {{ 5.0f, -0.5f, -5.0f},  {0.0f, 1.0f, 0.0f}, {2.0f, 2.0f}}
            };

            Render::OpenGL::Buffer vbo;
            vbo.Create();
            vbo.Storage(Core::MakeData(std::span{ vertices }));

            //Render::OpenGL::Buffer ebo;
            //ebo.Create();
            //ebo.Storage(Core::MakeData(std::span { indices }));

            constexpr Render::Attribute position_attribute{ 0, 3, Render::DataType::Float, false, 0 };
            constexpr Render::Attribute normal_attribute{ 1, 3, Render::DataType::Float, false, offsetof(Vertex, normal) };
            constexpr Render::Attribute uv_attribute{ 2, 2, Render::DataType::Float, false, offsetof(Vertex, uv) };

            _cubeVao.Create();
            _cubeVao.AttachVertices(vbo, sizeof(Vertex));
            //_cubeVao.AttachIndices(ebo);
            _cubeVao.Attribute(position_attribute);
            _cubeVao.Attribute(normal_attribute);
            _cubeVao.Attribute(uv_attribute);

            _lightSourceVao.Create();
            _lightSourceVao.AttachVertices(vbo, sizeof(Vertex));
            _lightSourceVao.Attribute(position_attribute);
            _lightSourceVao.Attribute(uv_attribute);

            Render::OpenGL::Buffer planeVbo;
            planeVbo.Create();
            planeVbo.Storage(Core::MakeData(std::span{ planeVertices }));

            _planeVao.Create();
            _planeVao.AttachVertices(planeVbo, sizeof(Vertex));
            _planeVao.Attribute(position_attribute);
            _planeVao.Attribute(normal_attribute);
            _planeVao.Attribute(uv_attribute);

            auto img = _imageLoader.Load("Textures/container2.png");
            _cubeDiffuseTexture.Create();
            _cubeDiffuseTexture.Upload(img);

            img = _imageLoader.Load("Textures/container2_specular.png");
            _cubeSpecularTexture.Create();
            _cubeSpecularTexture.Upload(img, {.colorSpace = Render::TextureColorSpace::Linear});

            img = _imageLoader.Load("Textures/metal.png");
            _metalAlbedo.Create();
            _metalAlbedo.Upload(img);

            Render::OpenGL::Commands::Clear(0.1f, 0.1f, 0.1f);

            glEnable(GL_DEPTH_TEST);

            glfwSetInputMode(WindowHandle(), GLFW_CURSOR, _lockCursor ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);

            lastX = (float)width() / 2.0f, lastY = (float)height() / 2.0f;

            _model = _modelLoader.Load("Models/nanosuit/nanosuit.obj");

            return true;
        }

        void onUpdate(float deltaTime) override
        {
            // Camera controls
            if (keys[GLFW_KEY_W])
                camera.ProcessKeyboard(FORWARD, deltaTime);
            if (keys[GLFW_KEY_S])
                camera.ProcessKeyboard(BACKWARD, deltaTime);
            if (keys[GLFW_KEY_A])
                camera.ProcessKeyboard(LEFT, deltaTime);
            if (keys[GLFW_KEY_D])
                camera.ProcessKeyboard(RIGHT, deltaTime);

            for (auto& light : lights)
            {
                if (light.type == LightType::Point)
                {
                    light.position.x = 1.0f + sin(glfwGetTime()) * 2.0f;
                    light.position.y = sin(glfwGetTime() / 2.0f) * 1.0f;
                }
                else if (light.type == LightType::Spot)
                {
                    light.position = camera.Position;
                    light.direction = camera.Front;
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
            Render::OpenGL::Commands::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glm::mat4 projection{ 1.0f };

            // rotate camera around (0, 0, 0)
            //GLfloat radius = 10.0f;
            //GLfloat camX = sin(glfwGetTime()) * radius;
            //GLfloat camZ = cos(glfwGetTime()) * radius;
            //view = glm::lookAt(glm::vec3(camX, 0.0, camZ), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));



            _pipeline.Bind();

            glm::mat4 view = camera.GetViewMatrix();
            projection = glm::perspective(glm::radians(45.0f), (GLfloat)width() / (GLfloat)height(), 0.1f, 100.0f);

            _pipeline.SetUniform("view", view);
            _pipeline.SetUniform("projection", projection);

            _pipeline.SetUniform("viewPos", camera.Position);

            _pipeline.SetUniform("material.diffuse", 0);
            _pipeline.SetUniform("material.specular", 1);
            _pipeline.SetUniform("material.shininess", 32.0f);

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
                    _pipeline.SetUniform("dirLight.direction", light.direction);
                    _pipeline.SetUniform("dirLight.ambient", light.ambient);
                    _pipeline.SetUniform("dirLight.diffuse", light.diffuse);
                    _pipeline.SetUniform("dirLight.specular", light.specular);
                }
                if (light.type == LightType::Point)
                {
                    _pipeline.SetUniform(std::format("pointLights[{}].position", pointLightIndex), light.position);
                    _pipeline.SetUniform(std::format("pointLights[{}].ambient", pointLightIndex), light.ambient);
                    _pipeline.SetUniform(std::format("pointLights[{}].diffuse", pointLightIndex), light.diffuse);
                    _pipeline.SetUniform(std::format("pointLights[{}].specular", pointLightIndex), light.specular);
                    _pipeline.SetUniform(std::format("pointLights[{}].constant", pointLightIndex), light.constant);
                    _pipeline.SetUniform(std::format("pointLights[{}].linear", pointLightIndex), light.linear);
                    _pipeline.SetUniform(std::format("pointLights[{}].quadratic", pointLightIndex), light.quadratic);
                    pointLightIndex++;
                }
                else if (light.type == LightType::Spot)
                {
                    _pipeline.SetUniform("spotLight.position", light.position);
                    _pipeline.SetUniform("spotLight.direction", light.direction);
                    _pipeline.SetUniform("spotLight.ambient", light.ambient);
                    _pipeline.SetUniform("spotLight.diffuse", light.diffuse);
                    _pipeline.SetUniform("spotLight.specular", light.specular);
                    _pipeline.SetUniform("spotLight.constant", light.constant);
                    _pipeline.SetUniform("spotLight.linear", light.linear);
                    _pipeline.SetUniform("spotLight.quadratic", light.quadratic);
                    _pipeline.SetUniform("spotLight.cutOff", light.cutOff);
                    _pipeline.SetUniform("spotLight.outerCutOff", light.outerCutOff);
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
            _cubeVao.Bind();

            glm::mat4 model = glm::mat4{ 1.0f };

            for (GLuint i = 0; i < 10; i++)
            {
                model = glm::translate(glm::mat4{ 1.0f }, cubePositions[i]);
                float angle = 20.0f * i;
                if (i % 3 == 0)  // every 3rd iteration (including the first) we set the angle using GLFW's time function.
                    angle = glfwGetTime() * 25.0f;
                model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
                _pipeline.SetUniform("model", model);

                Render::OpenGL::Commands::DrawVertices(Render::PrimitiveType::Triangles, 36);
            }

            // model
            {
                model = glm::scale(glm::mat4{ 1.0f }, glm::vec3(0.1f));
                _pipeline.SetUniform("model", model);
                _model.Render();
            }

            // floor
            {
                _planeVao.Bind();
                _metalAlbedo.Bind();
                glBindTextureUnit(1, 0);

                model = glm::mat4{ 1.0f };
                _pipeline.SetUniform("model", model);
                Render::OpenGL::Commands::DrawVertices(Render::PrimitiveType::Triangles, 6);
            }

            _lampPipeline.Bind();

            _lampPipeline.SetUniform("view", view);
            _lampPipeline.SetUniform("projection", projection);

            _lightSourceVao.Bind();
            for (unsigned int i = 0; i < lights.size(); i++)
            {
                const auto& light = lights[i];

                if (light.type == LightType::Point)
                {
                    model = glm::mat4(1.0f);
                    model = glm::translate(model, light.position);
                    model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
                    _lampPipeline.SetUniform("model", model);
                    _lampPipeline.SetUniform("lightColor", light.diffuse);
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

            camera.ProcessMouseMovement(xoffset, yoffset);
        }

    private:
        Assets::ImageLoader _imageLoader{};
        Assets::ObjModelLoader _modelLoader{};

        Render::Model _model;

        Render::OpenGL::Texture2D _cubeDiffuseTexture;
        Render::OpenGL::Texture2D _cubeSpecularTexture;
        Render::OpenGL::Texture2D _metalAlbedo;

        Render::OpenGL::VertexArray _cubeVao;
        Render::OpenGL::VertexArray _planeVao;
        Render::OpenGL::VertexArray _lightSourceVao;

        Render::OpenGL::Pipeline _pipeline;
        Render::OpenGL::Pipeline _lampPipeline;

        GLfloat lastX, lastY;
        bool firstMouse{ true };

        bool _isLineMode{};
        bool _lockCursor{ true };
    };
}