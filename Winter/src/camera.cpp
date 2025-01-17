#include "camera.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

glm::mat4 Camera::getViewMatrix() const
{
	const glm::mat4 view = glm::lookAt(_pos, _pos + _front, _up);

	// manual lookAt Code
	//const glm::mat4 rotation(
	//	_right.x, _up.x, -_front.x, 0.0f,
	//	_right.y, _up.y, -_front.y, 0.0f,
	//	_right.z, _up.z, -_front.z, 0.0f,
	//	0.0f, 0.0f, 0.0f, 1.0f
	//);
	//const glm::mat4 translation(
	//	1.0f, 0.0f, 0.0f, 0.0f,
	//	0.0f, 1.0f, 0.0f, 0.0f,
	//	0.0f, 0.0f, 1.0f, 0.0f,
	//	-_pos.x, _pos.y, -_pos.z, 1.0f
	//);
	//const glm::mat4 res = rotation * translation;
	return view;
}

glm::mat4 Camera::getProjectionMatrix(int width, int height) const
{
	const glm::mat4 projection = glm::perspective(glm::radians(_fov), static_cast<float>(width) / static_cast<float>(height), _nearPlane, _farPlane);
	return projection;
}