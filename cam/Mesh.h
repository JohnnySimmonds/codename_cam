#pragma once

#include "System.h"

#include <functional>
#include <memory>
#include <vector>

#include "Component.h"
#include "Events.h"
#include "ImageShader.h"
#include "ModelShader.h"
#include "Transform.h"
#include "Texture.h"

class Shader;

class Mesh : public Component {
public:
	Mesh(std::shared_ptr<Shader> shader,
		 const char* objFileName,
		 const char* texFileName,
		 glm::vec3 colour,
		 glm::vec3 scale,
		 GLuint type);

	// For Python... it can't handle the shared_ptr<Shader> from a ModelShader
	Mesh(std::shared_ptr<ModelShader> shader,
		const char* objFileName,
		const char* texFileName,
		glm::vec3 colour,
		glm::vec3 scale,
		GLuint type) : Mesh(std::shared_ptr<Shader>(shader), objFileName, texFileName, colour, scale, type) {}
	Mesh(std::shared_ptr<ImageShader> shader,
		const char* objFileName,
		const char* texFileName,
		glm::vec3 colour,
		glm::vec3 scale,
		GLuint type) : Mesh(std::shared_ptr<Shader>(shader), objFileName, texFileName, colour, scale, type) {}

	void GetMeshData(Events::Render event);
	void Destroy() override;

protected:
	void RegisterHandlers() override;

private:
	GLuint vao_;
	GLuint type_;
	GLuint count_;
	Texture texture_;

	std::shared_ptr<Shader> shader_;

	std::function<void(Events::Render)> handler_;
};
