#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera 
{
public:
	glm::vec3 position = { 0.0f, 5.0f, 0.0f };
	glm::vec3 front = { 0.0f, 0.0f, -1.0f };
	glm::vec3 up = { 0.0f, 1.0f, 0.0f };
	glm::vec3 right = { 1.0f, 0.0f, 0.0f };
	glm::vec3 worldUp = { 0.0f, 1.0f, 0.0f };

	float yaw = -90.0f;
	float pitch = -90.0f;

	float moveSpeed = 2.5f;
	float mouseSensitivity = 0.1f;

	Camera() { updateVectors(); }

	glm::mat4 getViewMatrix() const 
	{
		return glm::lookAt(position, position + front, up);
	}

	void processKeyboard(char key, float deltaTime) {
		float velocity = moveSpeed * deltaTime;
		if (key == 'W') position += front * velocity;
		if (key == 'S') position -= front * velocity;
		if (key == 'A') position -= right * velocity;
		if (key == 'D') position += right * velocity;
	}

	void processMouse(float xoffset, float yoffset) {
		xoffset *= mouseSensitivity;
		yoffset *= mouseSensitivity;

		yaw += xoffset;
		pitch += yoffset;

		pitch = glm::clamp(pitch, -89.0f, 89.0f);

		updateVectors();
	}

private:
	void updateVectors() {
		glm::vec3 f;
		f.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		f.y = sin(glm::radians(pitch));
		f.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		front = glm::normalize(f);
		right = glm::normalize(glm::cross(front, worldUp));
		up = glm::normalize(glm::cross(right, front));
	}
};
