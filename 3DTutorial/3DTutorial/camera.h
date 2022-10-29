#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

// For easier cross platform integration.
enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UP,
	DOWN
};

// Default camera values.
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2500.0f;
const float SENSITIVITY = 0.075f;
const float ZOOM = 45.0f;

class Camera
{
public:
	// camera variables.
	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;
	// Euler angles.
	float Yaw;
	float Pitch;
	// Camera options.
	float MovementSpeed;
	float MouseSensitivity;
	float Zoom;

	// Vector constructor.
	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
		float yaw = YAW,
		float pitch = PITCH);

	// Float constructor.
	Camera(float posX, float posY, float posZ,
		float upX, float upY, float upZ,
		float yaw = YAW,
		float pitch = PITCH);

	// Returns the view matrix calculated using Euler angles and the LookAt matrix.
	glm::mat4 GetViewMatrix();

	// processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems).
	void ProcessKeyboard(Camera_Movement direction, float deltaTime);

	// Processes input received from a mouse input system. Excepts the offset value in both x and y directions.
	void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);

	// Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis.
	void ProcessMouseScroll(float yoffset);

	// Calculates the front vector from the camera's (updated) Euler angles.
	void updateCameraVectors();
};

#endif