#pragma once

#include "LearnOpenGL-Shared.h"

// Sandbox headers
#include "App/SandboxApp.h"
#include "Assets/ImageLoader.h"
#include "Assets/ObjModelLoader.h"

namespace Eugenix
{
    // TODO : lights UBO from Eugenix. UBO Test. ShaderEnv stuff
	class LearnOpenGLAppBase : public SandboxApp
	{
	protected:
        void initCommon()
        {

        }

		Assets::ImageLoader _imageLoader{};
		Assets::ObjModelLoader _modelLoader{};

		Camera2 _camera{ glm::vec3(0.0f, 0.0f, 4.0f) };

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
                .direction = _camera.Front,
                .position = _camera.Position,
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
	};
}