#pragma once
#include <glm/glm.hpp>


namespace my_vulkan
{
	class Object;

	enum class CameraType { FIRST_PERSON, LOOK_AT };
	class Camera
	{
		friend class Object;
	public:
		float translateVelocity;
		float rotateVelocity;
		glm::vec3 position;
		glm::vec3 rotation;
		struct
		{
			glm::mat4 perspective;
			glm::mat4 view;
		} matrices;

		struct
		{
			glm::vec3 x;
			glm::vec3 y;
			glm::vec3 z;
		} axis;

		Camera(float fov, float aspect_ratio, const glm::vec3& position, const glm::vec3& rotation,
			CameraType type);
		Camera(float fov, float aspect_ratio, glm::vec3&& position , glm::vec3&& rotation ,
			CameraType type);

		void rotate(const glm::vec3& delta);
		void translate(const glm::vec3& delta);
		void moveForward();
		void moveBack();
		void moveLeft();
		void moveRight();
		void moveUp();
		void moveDown();
		void tick(glm::vec3 TranslationDelta, glm::vec3 rotationDelta);
	private:
		glm::vec3 orientation;
		glm::vec3 right;
		glm::vec3 up;
		float fov;
		float aspect_ratio;
		CameraType type;
		void updateMatrices();
	};
}


