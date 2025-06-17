#pragma once

#include <glm/glm.hpp>

namespace Eugenix::Scene
{
	struct Camera
	{
		Camera() 
			: position(0.0f, 0.0f, 0.0f)
			, yaw(-90.0f)
			, pitch(0.0f)
		{
			updateOrientation();
		}

		glm::vec3 position;
		float yaw;
		float pitch;

		glm::vec3 forward;
		glm::vec3 right;
		glm::vec3 up;

		void updateOrientation()
		{
			float yawRad = glm::radians(yaw);
			float pitchRad = glm::radians(pitch);

			forward.x = cos(pitchRad) * cos(yawRad);
			forward.y = sin(pitchRad);
			forward.z = cos(pitchRad) * sin(yawRad);
			forward = glm::normalize(forward);

			right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
			up = glm::normalize(glm::cross(right, forward));
		}
	};
} // namespace Eugenix::Scene