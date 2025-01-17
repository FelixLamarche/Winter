#pragma once

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"

class Camera
{
public:
	Camera(const glm::vec3& pos, const glm::vec3& front, const glm::vec3& up, const glm::vec3& rollYawPitch, float fov, float nearPlane, float farPlane)
		: _pos(pos), _front(front), _up(up), _right(glm::normalize(glm::cross(front, up))), 
		_roll(rollYawPitch.x), _yaw(rollYawPitch.y), _pitch(rollYawPitch.z),
			 _fov(fov),  _nearPlane(nearPlane), _farPlane(farPlane)
	{}

	glm::vec3 GetPosition() const { return _pos; }
	glm::vec3 GetFront() const { return _front; }
	glm::vec3 GetUp() const { return _up; }
	float GetPitch() const { return _pitch; }
	float GetYaw() const { return _yaw; }
	float GetRoll() const { return _roll; }
	float GetFOV() const { return _fov; }
	void SetPosition(const glm::vec3& position) { _pos = position; }
	void SetFront(const glm::vec3& front) { _front = front; }
	void SetUp(const glm::vec3& up) { _up = up; }
	void SetPitch(float pitch) { _pitch = pitch; SetFrontFromAngles(); }
	void SetYaw(float yaw) { _yaw = yaw; SetFrontFromAngles(); }
	void SetRoll(float roll) { _roll = roll; SetFrontFromAngles(); }
	void SetFOV(float fov) { _fov = fov; }
	glm::mat4 getViewMatrix() const;
	glm::mat4 getProjectionMatrix(int width, int height) const;
private:
	glm::vec3 _pos;
	glm::vec3 _front;
	glm::vec3 _up;
	glm::vec3 _right;
	float _roll;
	float _yaw;
	float _pitch;
	float _fov;
	float _nearPlane;
	float _farPlane;

	void SetFrontFromAngles() {
		_front.x = cos(glm::radians(_yaw)) * cos(glm::radians(_pitch));
		_front.y = sin(glm::radians(_pitch));
		_front.z = sin(glm::radians(_yaw)) * cos(glm::radians(_pitch));
		_front = glm::normalize(_front);
		// also re-calculate the Right and Up vector
		const glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
		// normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		_right = glm::normalize(glm::cross(_front, worldUp));  
		_up = glm::normalize(glm::cross(_right, _front));
	}
};