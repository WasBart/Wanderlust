#pragma once
#include <glm\glm.hpp>

namespace cgue
{
	namespace scene
	{
		class SceneObject
		{
		public:
			SceneObject(glm::mat4&modelMatrix);

			virtual ~SceneObject();

			virtual void update() = 0;
			virtual void draw() = 0;

			glm::mat4 modelMatrix;
		};
	}
}
#define SCENE_OBJECT