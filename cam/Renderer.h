#pragma once

#include "System.h"

#include <vector>

#include "Transform.h"

class Entity;

class Renderer
{
public:
	void Render(Entity &entity);

	static void Initialize();

	struct MeshData {
		GLuint vao;
		GLuint shader;
		GLuint count;
		GLenum type;
		mat4 modelMatrix;
	};

	GLFWwindow* getWindow();
private:
	static GLFWwindow *window_;
};
