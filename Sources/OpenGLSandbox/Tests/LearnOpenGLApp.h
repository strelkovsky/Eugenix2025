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
#include "Render/OpenGL/ShaderProgram.h"
#include "Render/OpenGL/Texture2D.h"
#include "Render/OpenGL/VertexArray.h"
#include "Render/OpenGL/Pipeline.h"
#include "Render/Types.h"

namespace Eugenix
{
    struct VertexPos
    {
        glm::vec3 pos;
    };

    struct Vertex
    {
        glm::vec3 pos;
        glm::vec3 normal;
        glm::vec2 uv;

        // TODO : return span of attributes
    };

    struct QuadVertex
    {
        glm::vec2 pos;
        glm::vec2 uv;
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

    class LearnOpenGLApp final : public SandboxApp
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
            cubemapTexture = loadCubemap(faces);

            _program = MakeProgramFromFiles("shaders/SimpleVertex.vert", "shaders/SimplePhong.frag");
            _lampProgram = MakeProgramFromFiles("shaders/SimpleVertex.vert", "shaders/SimpleUnlit.frag");
            _skyboxProgram = MakeProgramFromFiles("shaders/SimpleSkybox.vert", "shaders/SimpleSkybox.frag");

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

            const std::vector<Vertex> planeVertices = {
                {{-5.0f, -0.5f,  5.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
                {{ 5.0f, -0.5f,  5.0f}, {0.0f, 1.0f, 0.0f}, {2.0f, 0.0f}},
                {{-5.0f, -0.5f, -5.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 2.0f}},

                {{ 5.0f, -0.5f,  5.0f}, {0.0f, 1.0f, 0.0f}, {2.0f, 0.0f}},
                {{ 5.0f, -0.5f, -5.0f}, {0.0f, 1.0f, 0.0f}, {2.0f, 2.0f}},
                {{-5.0f, -0.5f, -5.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 2.0f}}
            };

            const std::vector<QuadVertex> quadVertices =
            { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
                // positions   // texCoords
                {{-1.0f,  1.0f},  {0.0f, 1.0f}},
                {{-1.0f, -1.0f},  {0.0f, 0.0f}},
                {{ 1.0f, -1.0f},  {1.0f, 0.0f}},
                                             
                {{-1.0f,  1.0f},  {0.0f, 1.0f}},
                {{ 1.0f, -1.0f},  {1.0f, 0.0f}},
                {{ 1.0f,  1.0f},  {1.0f, 1.0f}}
            };

            const std::vector<VertexPos> skyboxVertices = 
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

            Render::OpenGL::Buffer vbo;
            vbo.Create();
            vbo.Storage(Core::MakeData(std::span{ vertices }));

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

            // screen quad VAO
            Render::OpenGL::Buffer quadVbo;
            quadVbo.Create();
            quadVbo.Storage(Core::MakeData(std::span{ quadVertices }));

            _quadVao.Create();
            _quadVao.AttachVertices(quadVbo, sizeof(QuadVertex));
            _quadVao.Attribute({ 0, 2, Render::DataType::Float, false, 0 });
            _quadVao.Attribute({ 1, 2, Render::DataType::Float, false, offsetof(QuadVertex, uv) });

            // Skybox
            Render::OpenGL::Buffer skyboxVbo;
            skyboxVbo.Create();
            skyboxVbo.Storage(Core::MakeData(std::span{ skyboxVertices }));

            _skyboxVao.Create();
            _skyboxVao.AttachVertices(skyboxVbo, sizeof(VertexPos));
            _skyboxVao.Attribute({ 0, 3, Render::DataType::Float, false, 0 });

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
        }

        void onResize() override
        { 
        }

        void onRender() override
        {
            Render::OpenGL::Pipeline::Enable(Render::PipelineFeature::DepthTest);
            Render::OpenGL::Commands::Viewport(0, 0, width(), height());
            Render::OpenGL::Commands::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            drawScene();

            //_skyboxPipeline.SetUniform("skybox", 0);

            // draw skybox as last
            glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
            _skyboxProgram.Bind();
            glm::mat4 view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
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

            _program.Bind();

            glm::mat4 view = camera.GetViewMatrix();
            projection = glm::perspective(glm::radians(45.0f), (GLfloat)width() / (GLfloat)height(), 0.1f, 100.0f);

            _program.SetUniform("view", view);
            _program.SetUniform("projection", projection);

            _program.SetUniform("viewPos", camera.Position);

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
        Render::OpenGL::VertexArray _quadVao;
        Render::OpenGL::VertexArray _skyboxVao;

        Render::OpenGL::ShaderProgram _program;
        Render::OpenGL::ShaderProgram _lampProgram;
        Render::OpenGL::ShaderProgram _skyboxProgram;

        GLfloat lastX, lastY;
        bool firstMouse{ true };

        bool _lockCursor{ true };
    };
}