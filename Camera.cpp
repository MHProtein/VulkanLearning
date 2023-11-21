#define GLM_FORCE_RADIANCE
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "Camera.h"

#include <algorithm>
#include <iostream>

#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtx/io.hpp"
#include "glm/gtx/scalar_multiplication.hpp"

my_vulkan::Camera::Camera(float fov, float aspect_ratio, const glm::vec3& position, const glm::vec3& rotation, CameraType type)
{
	this->fov = fov;
	this->aspect_ratio = aspect_ratio;
	this->position = position;
	this->rotation = rotation;
	this->type = type;
	translateVelocity = 0.01f;
	rotateVelocity = 1.0f;
	matrices.perspective = glm::perspective(fov, aspect_ratio, 0.1f, 1000.0f);
	axis.x = { 1, 0, 0 };
	axis.y = { 0, 1, 0 };
	axis.z = { 0, 0, 1 };
	updateMatrices();
}

my_vulkan::Camera::Camera(float fov, float aspect_ratio, glm::vec3&& position, glm::vec3&& rotation, CameraType type)
{
	this->fov = fov;
	this->aspect_ratio = aspect_ratio;
	this->position = position;
	this->rotation = rotation;
	this->type = type;
	matrices.perspective = glm::perspective(fov, aspect_ratio, 0.1f, 1000.0f);
	axis.x = { 1, 0, 0 };
	axis.y = { 0, 1, 0 };
	axis.z = { 0, 0, 1 };
	updateMatrices();
}

void my_vulkan::Camera::rotate(const glm::vec3& delta)
{
	rotation += delta;
	if (rotation.x - 360.0f > 0.01f)
		rotation.x = 0.0f;
	if (rotation.y - 360.0f > 0.01f)
		rotation.y = 0.0f;
	if (rotation.z - 360.0f > 0.01f)
		rotation.z = 0.0f;

	if(rotation.x < 0.0f)
		rotation.x = 360.0f;
	if (rotation.y < 0.0f)
		rotation.y = 360.0f;
	if (rotation.z < 0.0f)
		rotation.z = 360.0f;
	updateMatrices();
}

void my_vulkan::Camera::translate(const glm::vec3& delta)
{
	position += delta;
	updateMatrices();
}

void my_vulkan::Camera::moveForward()
{
	orientation *= translateVelocity;
	translate(orientation);
	updateMatrices();
}

void my_vulkan::Camera::moveBack()
{
	orientation *= translateVelocity;
	translate(-orientation);
	updateMatrices();
}

void my_vulkan::Camera::moveLeft()
{
	translate(-right * translateVelocity);
	updateMatrices();
}

void my_vulkan::Camera::moveRight()
{
	translate(right * translateVelocity);
	updateMatrices();
}

void my_vulkan::Camera::moveUp()
{
	translate(up * translateVelocity);
	updateMatrices();
}

void my_vulkan::Camera::moveDown()
{
	translate(-up * translateVelocity);
	updateMatrices();
}

void my_vulkan::Camera::tick(glm::vec3 TranslationDelta, glm::vec3 rotationDelta)
{
	translate(TranslationDelta);
	rotate(rotationDelta);
}

void my_vulkan::Camera::updateMatrices()
{
	glm::mat4 rotation_matrix = glm::mat4(1.0f);
	glm::mat4 transformation_matrix;

	//DONT CHANGE THE FUCKING ORDER
	rotation_matrix = glm::rotate(rotation_matrix, glm::radians(this->rotation.y) * rotateVelocity, glm::normalize(axis.y));
	rotation_matrix = glm::rotate(rotation_matrix, glm::radians(this->rotation.z) * rotateVelocity, glm::normalize(axis.z));
	rotation_matrix = glm::rotate(rotation_matrix, glm::radians(this->rotation.x) * rotateVelocity, glm::normalize(axis.x));
	transformation_matrix = glm::translate(this->position);
	matrices.view = glm::inverse(transformation_matrix * rotation_matrix);

	orientation.x = -matrices.view[0][2];
	orientation.y = -matrices.view[1][2];
	orientation.z = -matrices.view[2][2];
	normalize(orientation);

	right.x = matrices.view[0][0];
	right.y = matrices.view[1][0];
	right.z = matrices.view[2][0];
	normalize(right);

	up.x = matrices.view[0][1];
	up.y = matrices.view[1][1];
	up.z = matrices.view[2][1];
	normalize(up);

}
