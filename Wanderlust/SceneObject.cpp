#include "SceneObject.h"

using namespace cgue::scene; 

SceneObject::SceneObject(glm::mat4&_modelMatrix)
	:modelMatrix(_modelMatrix)
{

}

SceneObject::~SceneObject(){

}