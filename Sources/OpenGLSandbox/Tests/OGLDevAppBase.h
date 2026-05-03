#pragma once

#include <chrono>

#include "OGLDevAssimp.h"
#include "OGLDevBaseMesh.h"
#include "OGLDevCamera.h"
#include "OGLDevCommon.h"
#include "OGLDevMath.h"
#include "OGLDevLighting.h"

#include "App/SandboxApp.h"
#include "Assets/ImageLoader.h"
#include "Render/OpenGL/Buffer.h"
#include "Render/OpenGL/Commands.h"
#include "Render/OpenGL/Sampler.h"
#include "Render/OpenGL/ShaderProgram.h"
#include "Render/OpenGL/Texture2D.h"
#include "Render/OpenGL/VertexArray.h"

using clock_type = std::chrono::steady_clock;

namespace Eugenix
{
    constexpr uint32_t OGLDEV_MAX_POINT_LIGHTS = 2;

    constexpr float ATTEN_STEP = 0.01f;

    class OGLDevAppBase final : public SandboxApp
    {
    public:
        OGLDevAppBase() : SandboxApp(1920, 1080) 
            , _camera{ { 0.0f, 4.0f, 10.0f } }
        {}

    protected:
        bool onInit() override
        {
            createGeometry();
            createShaderProgram();
            createSamplers();

            Render::OpenGL::Commands::Clear(0.3f, 0.3f, 0.3f);
            // когда у вас есть замкнутая поверхность, нужно включить отбраковку граней, чтобы включить
            // только внешние части поверхностей, а не треугольники, которые направлены внутрь.
            // треугольники, направленные вперед, должны быть отрисованы по часовой стрелке (???).
            glEnable(GL_CULL_FACE);
            glFrontFace(GL_CCW);
            glCullFace(GL_BACK);

            glEnable(GL_DEPTH_TEST);

            dirLight.ambientIntensity = 0.05f;
            dirLight.diffuseIntensity = 0.3f;
            dirLight.worldDirection = glm::vec3(1.0f, 0.0f, 0.0f);
            dirLight.color = glm::vec3(1.0f, 1.0f, 1.0f);

            pointLights[0].ambientIntensity = 0.5f;
            pointLights[0].diffuseIntensity = 1.0f;
            pointLights[0].color = glm::vec3(1.0f, 1.0f, 1.0f);
            pointLights[0].attenuation.linear = 0.2f;
            pointLights[0].attenuation.exp = 0.0f;

            pointLights[1].diffuseIntensity = 1.0f;
            pointLights[1].color = glm::vec3(1.0f, 0.0f, 0.0f);
            pointLights[1].attenuation.linear = 0.0f;
            pointLights[1].attenuation.exp = 0.2f;

            return true;
        }

        void onUpdate(float deltaTime) override
        {
            _time += deltaTime;

            AngleInRadians += deltaTime * DeltaSign;
            if ((Loc >= 0.5f) || (Loc <= -0.4f))
                DeltaSign *= -1;

            PersProjInfo perspInfo = { 45.0f, width(), height(), 0.1f, 100.0f };
            projection = Perspective(perspInfo);

            pointLights[0].worldPosition.x = -2.5f;
            pointLights[0].worldPosition.y = sinf(_time) * 4 + 4;
            pointLights[0].worldPosition.z = 0.0f;

            pointLights[1].worldPosition.x = 2.5f;
            pointLights[1].worldPosition.y = sinf(_time) * 4 + 4;
            pointLights[1].worldPosition.z = 0.0f;

            _cameraController.ProcessKeyboard(WindowHandle(), _camera, deltaTime);
        }

        void uploadDirectionalLight(const Eugenix::Render::OpenGL::ShaderProgram& shaderProgram, WorldTransform& worldTransform)
        {
            shaderProgram.SetUniform("gDirectionalLight.Base.Color", dirLight.color);
            shaderProgram.SetUniform("gDirectionalLight.Base.AmbientIntensity", dirLight.ambientIntensity);
            shaderProgram.SetUniform("gDirectionalLight.Base.DiffuseIntensity", dirLight.diffuseIntensity);
            shaderProgram.SetUniform("gDirectionalLight.Direction", glm::normalize(dirLight.worldDirection));
        }

        void uploadMaterial(const OGLDevMaterial& material, const Eugenix::Render::OpenGL::ShaderProgram& shaderProgram)
        {
            shaderProgram.SetUniform("gMaterial.AmbientColor", material.ambientColor);
            shaderProgram.SetUniform("gMaterial.DiffuseColor", material.diffuseColor);
            shaderProgram.SetUniform("gMaterial.SpecularColor", material.specularColor);
            shaderProgram.SetUniform("gMaterial.Shininess", 32.0f);
        }

        void uploadPointLights(const Eugenix::Render::OpenGL::ShaderProgram& shaderProgram, WorldTransform& worldTransform)
        {
            constexpr int numLights = OGLDEV_MAX_POINT_LIGHTS;
            shaderProgram.SetUniform("gNumPointLights", numLights);

            for (unsigned int i = 0; i < numLights; i++)
            {
                char Name[128];
                memset(Name, 0, sizeof(Name));

                sprintf_s(Name, sizeof(Name), "gPointLights[%d].Base.Color", i);
                shaderProgram.SetUniform(Name, pointLights[i].color);

                sprintf_s(Name, sizeof(Name), "gPointLights[%d].Base.AmbientIntensity", i);
                shaderProgram.SetUniform(Name, pointLights[i].ambientIntensity);

                sprintf_s(Name, sizeof(Name), "gPointLights[%d].Base.DiffuseIntensity", i);
                shaderProgram.SetUniform(Name, pointLights[i].diffuseIntensity);

                sprintf_s(Name, sizeof(Name), "gPointLights[%d].WorldPos", i);
                shaderProgram.SetUniform(Name, pointLights[i].worldPosition);

                sprintf_s(Name, sizeof(Name), "gPointLights[%d].Atten.Constant", i);
                shaderProgram.SetUniform(Name, pointLights[i].attenuation.constant);

                sprintf_s(Name, sizeof(Name), "gPointLights[%d].Atten.Linear", i);
                shaderProgram.SetUniform(Name, pointLights[i].attenuation.linear);

                sprintf_s(Name, sizeof(Name), "gPointLights[%d].Atten.Exp", i);
                shaderProgram.SetUniform(Name, pointLights[i].attenuation.exp);
            }
        }

        void uploadPerObjectUniforms(BasicMesh& mesh, const Eugenix::Render::OpenGL::ShaderProgram& shaderProgram, WorldTransform& worldTransform, const glm::mat4& mvp)
        {
            const glm::mat4 world = worldTransform.GetMatrix();
            const glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(world)));

            shaderProgram.SetUniform("gWorld", world);
            shaderProgram.SetUniform("gMVP", mvp);
            shaderProgram.SetUniform("gNormalMatrix", normalMatrix);

            shaderProgram.SetUniform("gViewPos", _camera.GetPosition());

            uploadDirectionalLight(_shaderProgram, worldTransform);
            uploadMaterial(mesh.GetMaterial(), _shaderProgram);
            uploadPointLights(_shaderProgram, worldTransform);
            //uploadSpotLights(_shaderProgram, worldTransform);
        }

        void renderMesh(BasicMesh& mesh, const glm::vec3& position, const glm::vec3& rotation, float scale)
        {
            WorldTransform& worldTransform = mesh.GetWorldTransform();

            worldTransform.SetScale(scale);
            worldTransform.SetPosition(position);
            worldTransform.SetRotation(rotation.x, rotation.y, rotation.z);

            const auto view = _camera.GetViewMatrix();
            const auto mvp = projection * view * worldTransform.GetMatrix();

            uploadPerObjectUniforms(mesh, _shaderProgram, worldTransform, mvp);

            mesh.Render();
        }

        void onRender() override
        {
            Render::OpenGL::Commands::Viewport(0, 0, width(), height());
            Render::OpenGL::Commands::Clear(Render::ClearFlags::Color | Render::ClearFlags::Depth);

            _modelSampler.Bind(0);
            _shaderProgram.Bind();

            
            renderMesh(_mesh, { 0.0f, -0.5f, 0.0f }, {}, 1.0f);
            renderMesh(_mesh2, { 0.0f, 0.0f, 0.0f }, {glm::radians(-90.0f), 0.0f, 0.0f}, 1.0f);
        }

        void onKeyHandle(int key, int code, int action, int mode) override
        {
            if (action == GLFW_PRESS)
            {
                switch (key)
                {
                case GLFW_KEY_1:
                    pointLights[0].attenuation.linear += ATTEN_STEP;
                    pointLights[1].attenuation.linear += ATTEN_STEP;
                    break;

                case GLFW_KEY_2:
                    pointLights[0].attenuation.linear -= ATTEN_STEP;
                    pointLights[1].attenuation.linear -= ATTEN_STEP;
                    break;

                case GLFW_KEY_3:
                    pointLights[0].attenuation.exp += ATTEN_STEP;
                    pointLights[1].attenuation.exp += ATTEN_STEP;
                    break;

                case GLFW_KEY_4:
                    pointLights[0].attenuation.exp -= ATTEN_STEP;
                    pointLights[1].attenuation.exp -= ATTEN_STEP;
                    break;
                }
            }
        }

        void onMouseHandle(double xPos, double yPos) override
        {
            //_camera.ProceedMouse(xPos, yPos);
            // TODO : перенести в Update(?) после того, как будет выделен модуль Platform с инпутом.
            _cameraController.ProcessMouse(_camera, xPos, yPos);
        }

        void onMouseButtonHandle(int button, int action, int mods) override
        {
            if (button == GLFW_MOUSE_BUTTON_LEFT)
            {
            }
        }

    private:
        void createGeometry()
        {
            _mesh.LoadMesh("models/box_terrain/box_terrain.obj");
            _mesh2.LoadMesh("models/Vanguard.dae");
        }

        void createShaderProgram()
        {
            _shaderProgram = MakeProgramFromFiles("Shaders/phong-lighting4-spot-lights-world-space.vert", "Shaders/phong-lighting4-spot-lights-world-space.frag");
        }

        void createSamplers()
        {
            _modelSampler.Create();
            _modelSampler.Parameter(Render::TextureParam::MinFilter, Render::TextureFilter::Linear);
            _modelSampler.Parameter(Render::TextureParam::MagFilter, Render::TextureFilter::Linear);
            _modelSampler.Parameter(Render::TextureParam::WrapS, Render::TextureWrapping::Repeat);
            _modelSampler.Parameter(Render::TextureParam::WrapT, Render::TextureWrapping::Repeat);
        }

        Render::OpenGL::ShaderProgram _shaderProgram;

        Render::OpenGL::Sampler _modelSampler;

        float Scale = 0.5f;
        float AngleInRadians = 0.0f;
        float Loc = 0.0f;
        float DeltaSign = 1.0f;

        Assets::ImageLoader _imageLoader{};

        glm::mat4 projection{ 1.0f };
        Camera3 _camera;
        CameraController _cameraController;

        DirectionalLight dirLight;
        PointLight pointLights[OGLDEV_MAX_POINT_LIGHTS];

        BasicMesh _mesh;
        BasicMesh _mesh2;

        float _time = 0.0f;
    };
}
