#pragma once

#include <GLFW/glfw3.h>

#include "OGLDevMath.h"

struct Camera3
{
    Camera3(const glm::vec3& position = { 0.0f, 0.0f, 1.0f })
        : _position{ position }
        , _up{ 0.0f, 1.0f, 0.0f }
        , _yaw{ -90.0f }
        , _pitch{ 0.0f }
    {
        _worldUp = { 0.0f, 1.0f, 0.0f };
        UpdateVectors();
    }

    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(_position, _position + _forward, _up);
    }

    //void ProceedKey(int key)
    //{
    //    glm::vec3 right = glm::normalize(glm::cross(_forward, _up));
    //    glm::vec3 left = -right;

    //    const float step = _speed /* * deltaTime*/;

    //    switch (key)
    //    {
    //    case GLFW_KEY_W:
    //        _position += _forward * step;
    //        break;

    //    case GLFW_KEY_S:
    //        _position -= _forward * step;
    //        break;

    //    case GLFW_KEY_A:
    //        _position += left * step;
    //        break;

    //    case GLFW_KEY_D:
    //        _position += right * step;
    //        break;

    //    case GLFW_KEY_Q:
    //        _position.y += step;
    //        break;

    //    case GLFW_KEY_E:
    //        _position.y -= step;
    //        break;

    //    default:
    //        break;
    //    }
    //}

    //void ProceedMouse(double xpos, double ypos)
    //{
    //    if (_firstMouse)
    //    {
    //        _lastMouseX = xpos;
    //        _lastMouseY = ypos;
    //        _firstMouse = false;
    //        return;
    //    }

    //    double xoffset = xpos - _lastMouseX;
    //    double yoffset = _lastMouseY - ypos;

    //    _lastMouseX = xpos;
    //    _lastMouseY = ypos;

    //    xoffset *= _sensitivity;
    //    yoffset *= _sensitivity;

    //    _yaw += static_cast<float>(xoffset);
    //    _pitch += static_cast<float>(yoffset);
    //    _pitch = glm::clamp(_pitch, -89.0f, 89.0f);

    //    UpdateVectors();
    //}

    //void ResetMouseTracking()
    //{
    //    _firstMouse = true;
    //}

    void OffsetYawPitch(float yawOffsetDeg, float pitchOffsetDeg)
    {
        _yaw += yawOffsetDeg;
        _pitch += pitchOffsetDeg;
        _pitch = std::clamp(_pitch, -89.0f, 89.0f);
        UpdateVectors();
    }

    void MoveForward(float distance)
    {
        _position += _forward * distance;
    }

    void MoveRight(float distance)
    {
        _position += _right * distance;
    }

    void MoveUp(float distance)
    {
        _position += _worldUp * distance;
    }

private:
    void UpdateVectors()
    {
        glm::vec3 direction;
        direction.x = cos(glm::radians(_yaw)) * cos(glm::radians(_pitch));
        direction.y = sin(glm::radians(_pitch));
        direction.z = sin(glm::radians(_yaw)) * cos(glm::radians(_pitch));

        _forward = glm::normalize(direction);
        _right   = glm::normalize(glm::cross(_forward, glm::vec3(0.0f, 1.0f, 0.0f)));
        _up      = glm::normalize(glm::cross(_right, _forward));
    }

    glm::vec3 _position; // World-space position
    glm::vec3 _forward;
    glm::vec3 _right;
    glm::vec3 _up;
    glm::vec3 _worldUp;

    float _yaw{ -90.0f };
    float _pitch{ 0.0f };
};

struct CameraController
{
    //Если потом будешь переключать режим “мышь для UI / мышь для камеры”, полезно вызывать:
    //после повторного включения камеры, чтобы не было резкого скачка на первом движении.
    auto ResetMouseTracking() -> void
    {
        _firstMouse = true;
    }

    auto ProcessKeyboard(GLFWwindow* window, Camera3& camera, float deltaTime) -> void
    {
        const float distance = _mouseSpeed * deltaTime;

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        {
            camera.MoveForward(distance);
        }

        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        {
            camera.MoveForward(-distance);
        }

        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        {
            camera.MoveRight(-distance);
        }

        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        {
            camera.MoveRight(distance);
        }

        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        {
            camera.MoveUp(distance);
        }

        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        {
            camera.MoveUp(-distance);
        }
    }

    auto ProcessMouse(Camera3& camera, double xpos, double ypos) -> void
    {
        if (_firstMouse)
        {
            _lastX = xpos;
            _lastY = ypos;
            _firstMouse = false;
            return;
        }

        double xoffset = xpos - _lastX;
        double yoffset = _lastY - ypos;

        _lastX = xpos;
        _lastY = ypos;

        xoffset *= _mouseSensitivity;
        yoffset *= _mouseSensitivity;

        camera.OffsetYawPitch(static_cast<float>(xoffset), static_cast<float>(yoffset));
    }
private:
    float _mouseSpeed{ 3.0f };
    float _mouseSensitivity{ 0.1f };

    double _lastX{ 0.0 };
    double _lastY{ 0.0 };
    bool _firstMouse{ true };
};

//FreeLookCameraController
//OrbitCameraController
//FlyCameraController
//TopDownCameraController
//EditorCameraController
//RTSCameraController
struct ICameraController
{
    virtual ~ICameraController() = default;

    virtual void Update(Camera3& camera, float deltaTime) = 0;
    virtual void ProcessMouse(Camera3& camera, double xpos, double ypos) = 0;
    virtual void ResetMouseTracking() = 0;
};

class FreeLookCameraController final : public ICameraController
{
};