#pragma once

#include <glew/glew.h>
#include "SceneObject.h"
#include "shader.h"
#include "glm\glm.hpp"

#ifdef SCENE_OBJECT

#define CUBE_VERTEX_COUNT 8
#define CUBE_INDEX_COUNT 36

namespace cgue
{
	namespace scene
	{
		class Cube : public SceneObject
		{
		public:
			Cube(glm::mat4& matrix, Shader* _shader);
			virtual ~Cube();

			virtual void update();
			virtual void draw();

		private:
			GLuint vao;
			GLuint positionBuffer, indexBuffer; 
			Shader* shader;

			static const float positions[CUBE_VERTEX_COUNT*3];
			static const unsigned int indices[CUBE_INDEX_COUNT];
		};
	}
}
#endif