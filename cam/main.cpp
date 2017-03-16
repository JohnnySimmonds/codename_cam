#include "Audio.h" //has to be at the top for some reason
#include "Controller.h"

#include <Windows.h>
#include <MMSystem.h>
#include <iostream>

#include "Camera.h"
#include "Entity.h"
#include "Image.h"
#include "Mesh.h"
#include "ModelShader.h"
#include "NavMesh.h"
#include "Physics.h"
#include "Renderer.h"
#include "RigidBody.h"
#include "ScriptComponent.h"
#include "Shader.h"
#include "Vehicle.h"
/*Not sure where to put this afterwards just for testing*/
#include "drawAiPath.h"

int main() {
	try {
		Renderer::Initialize();
		Renderer renderer;
		Physics physics;
		auto root = Entity::Create();

#ifdef DEBUG

		std::function<void(Events::Render)> handler = [&physics](Events::Render e) {
			auto d = physics.GetDebugMeshData();
			e.data.insert(e.data.end(), d.begin(), d.end());
		};

		root->RegisterEventHandler(&handler);

		//drawAiPath drawPath;

		//std::function<void(Events::Render)> pathHandle = [&drawPath](Events::Render b) {
		//	auto d = drawPath.GetDebugMeshData();
		//	b.data.insert(b.data.end(), d.begin(), d.end());
		//};
		//root->RegisterEventHandler(&pathHandle);

#endif // #ifdef DEBUG


		Audio audio;

		//audio.initAudio();
		//audio.playAudio(4); //1,2,3,4 for Audio atm can play more than one at a time



		{
			auto plane = Entity::Create(root.get()).lock();
			std::shared_ptr<Component> planemesh(new Mesh(std::unique_ptr<Shader>(new ModelShader), "map_mesh.fbx", vec3(0.2, 0.4, 0.2), vec3(1.0f, 1.0f, 1.0f), GL_TRIANGLES));
			std::shared_ptr<Component> planebody(new RigidBody(physics, *physics.GetPhysics()->createMaterial(1.0f, 1.0f, 1.0f), "map_mesh.fbx", 1.0f, false));
			plane->AddComponent(std::move(planemesh));
			plane->AddComponent(std::move(planebody));

			auto vehicle = Entity::Create(root.get()).lock();

			std::shared_ptr<Component> mesh(new Mesh(std::unique_ptr<Shader>(new ModelShader), "chaser_mesh.obj", vec3(0.1, 0.1, 0.6), vec3(1.0f, 1.0f, 1.0f), GL_TRIANGLES));
			std::shared_ptr<Component> v(new ScriptComponent("vehicle", physics));
			std::shared_ptr<Component> c(new ScriptComponent("collision", physics));
			vehicle->AddComponent(std::move(mesh));
			vehicle->AddComponent(std::move(v));
			vehicle->AddComponent(std::move(c));
			vehicle->AddComponent(std::make_unique<ScriptComponent>("chaser", physics));

			auto aiVehicle = Entity::Create(root.get()).lock();
			std::shared_ptr<Component> aiMesh(new Mesh(std::unique_ptr<Shader>(new ModelShader), "runner_mesh.obj", vec3(1.0, 0.84, 0.0), vec3(1.5f, 1.5f, 1.5f), GL_TRIANGLES));//debug seems to work better was 2.5
			std::shared_ptr<Component> aiV(new ScriptComponent("chaser_ai", physics));
			aiVehicle->AddComponent(std::move(aiMesh));
			aiVehicle->AddComponent(std::move(aiV));

			aiVehicle->AddComponent(std::make_unique<ScriptComponent>("runner", physics));

			auto smiley = Entity::Create(root.get()).lock();
			std::shared_ptr<Component> image(new Image("smiley.png"));
			smiley->AddComponent(std::move(image));
		}



		auto lastTime = timer::now();

		bool soundT = true;
		while (!glfwWindowShouldClose(renderer.getWindow())) {
			auto currentTime = timer::now();
			auto dt = seconds(currentTime - lastTime);
			lastTime = currentTime;
			
			physics.Update(dt);
			root->Update(dt);
			renderer.Render(*root);
			//audio.playAudio(4);
			Entity::DeleteDestroyed();

			glfwPollEvents();

		}

		return 0;
	}
	catch (const std::runtime_error &e) {
		std::cout << "Fatal error: " << e.what() << "\n";
		return 1;
	}
	catch (...) {
		std::cout << "Unrecognized fatal error\n";
		return 1;
	}
}
