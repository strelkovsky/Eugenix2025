#pragma once

#include <glm/glm.hpp>

namespace Eugenix::Math
{
	struct Transform
	{
		void Translate(const glm::vec3& translate)
		{
			_modelMatrix = glm::translate(_modelMatrix, translate);
		}

		// in degrees
		void Rotate(const glm::vec3& rotation)
		{
			_modelMatrix = glm::rotate(_modelMatrix, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
			_modelMatrix = glm::rotate(_modelMatrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
			_modelMatrix = glm::rotate(_modelMatrix, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
		}

		void Scale(const glm::vec3& scale)
		{
			_modelMatrix = glm::scale(_modelMatrix, scale);
		}

		void Reset()
		{
			_modelMatrix = glm::mat4{ 1.0f };
		}

		const glm::mat4& Matrix() const
		{
			return _modelMatrix;
		}
	private:
		glm::mat4 _modelMatrix{ 1.0f };
	};
} // Eugenix::Math