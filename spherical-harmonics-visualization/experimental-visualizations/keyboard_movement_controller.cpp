#include "keyboard_movement_controller.hpp"

// std
#include <limits>

#include <iostream>


namespace vae {
	void KeyboardMovementController::moveInPlaneXZ(GLFWwindow* window, float dt, VvtGameObject& gameObject)
	{
		// ROTATION IN XZ PLANE
		glm::vec3 rotate{ 0 };
		if (glfwGetKey(window, keys.lookRight) == GLFW_PRESS)rotate.y += 1.f;
		if (glfwGetKey(window, keys.lookLeft) == GLFW_PRESS)rotate.y -= 1.f;
		if (glfwGetKey(window, keys.lookUp) == GLFW_PRESS)rotate.x += 1.f;
		if (glfwGetKey(window, keys.lookDown) == GLFW_PRESS)rotate.x -= 1.f;

		// Check if rotation is not zero
		if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
			// Rotation speed depends on lookspeed and time that has passed since last frame!
			gameObject.transform.rotation += lookSpeed * dt * glm::normalize(rotate);
		}

		gameObject.transform.rotation.x = glm::clamp(gameObject.transform.rotation.x, -1.5f, 1.5f);
		gameObject.transform.rotation.y = glm::mod(gameObject.transform.rotation.y, glm::two_pi<float>());

		// TRANSLATION
		float yaw = gameObject.transform.rotation.y;
		const glm::vec3 forwardDir(sin(yaw), 0.f, cos(yaw));
		const glm::vec3 rightDir(forwardDir.z, 0.f, -forwardDir.x);
		const glm::vec3 upDir(0.f, -1.f, 0.f);

		glm::vec3 moveDir{ 0.f };
		if (glfwGetKey(window, keys.moveForward) == GLFW_PRESS)moveDir += forwardDir;
		if (glfwGetKey(window, keys.moveBackward) == GLFW_PRESS)moveDir -= forwardDir;
		if (glfwGetKey(window, keys.moveRight) == GLFW_PRESS)moveDir += rightDir;
		if (glfwGetKey(window, keys.moveLeft) == GLFW_PRESS)moveDir -= rightDir;
		if (glfwGetKey(window, keys.moveUp) == GLFW_PRESS)moveDir += upDir;
		if (glfwGetKey(window, keys.moveDown) == GLFW_PRESS)moveDir -= upDir;

		// Check if move direction is not zero
		if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) {
			// Rotation speed depends on lookspeed and time that has passed since last frame!
			gameObject.transform.translation += moveSpeed * dt * glm::normalize(moveDir);
		}
	}



}