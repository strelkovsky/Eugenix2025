#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Этот класс включает в себя все преобразования, необходимые для перехода от локальной
// системы координат объекта к мировой системе координат.
class WorldTransform
{
public:
    WorldTransform() {}

    void SetScale(float scale)
    {
        m_scale = scale;
    }
    // лучше прямо зафиксировать это в API: либо SetRotationRadians, либо SetRotationDegrees
    void SetRotation(float x, float y, float z)
    {
        m_rotation.x = x;
        m_rotation.y = y;
        m_rotation.z = z;
    }
    void SetPosition(float x, float y, float z)
    {
        m_pos.x = x;
        m_pos.y = y;
        m_pos.z = z;
    }

    void Rotate(float x, float y, float z)
    {
        m_rotation.x += x;
        m_rotation.y += y;
        m_rotation.z += z;
    }

    glm::mat4 GetMatrix()
    {
        glm::mat4 worldTransformMat{ 1.0f };

        worldTransformMat = glm::translate(worldTransformMat, m_pos);
        //Если хочешь сделать «как в норм движке»
        //Ты сейчас используешь Euler rotation → это ок, но:
        //будут проблемы с gimbal lock
        //неудобно для камеры
        //Я бы предложил следующий шаг :
        //glm::quat q = glm::quat(glm::radians(m_rotation));
        //glm::mat4 rotation = glm::mat4_cast(q);
        worldTransformMat = glm::rotate(worldTransformMat, m_rotation.x, { 1,0,0 });
        worldTransformMat = glm::rotate(worldTransformMat, m_rotation.y, { 0,1,0 });
        worldTransformMat = glm::rotate(worldTransformMat, m_rotation.z, { 0,0,1 });
        worldTransformMat = glm::scale(worldTransformMat, glm::vec3(m_scale));

        return worldTransformMat;
    }

private:
    float    m_scale = 1.0f;
    glm::vec3 m_rotation = glm::vec3(0.0f, 0.0f, 0.0f); // In radians !!!!!
    glm::vec3 m_pos = glm::vec3(0.0f, 0.0f, 0.0f);
};

struct PersProjInfo
{
    float FOV = 0.0f;
    float Width = 0.0f;
    float Height = 0.0f;
    float zNear = 0.0f;
    float zFar = 0.0f;
};

inline glm::mat4 Perspective(const PersProjInfo& perspInfo)
{
    return glm::perspective(
        glm::radians(perspInfo.FOV),
        static_cast<float>(perspInfo.Width) / static_cast<float>(perspInfo.Height),
        perspInfo.zNear,
        perspInfo.zFar
    );
}