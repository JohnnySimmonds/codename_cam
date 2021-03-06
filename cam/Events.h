#pragma once

#include <boost/python.hpp>

#include "EventSystem.h"
#include "Renderer.h"
#include "Trigger.h"
#include "Audio.h"
class Entity;

using namespace boost;

namespace Events {

	struct Sound {
		std::vector<ALuint> &sources;
		std::vector<int> &choice;
		std::vector<int> &pChoice;
		std::vector<vec3> &soundPosition;
		std::vector<bool> &isPlayer;
		std::vector<vec3> &forwardVecs;
	};
	struct Infected {
		Entity* other;
		Entity* GetOther() { return other; }
	};
	struct Revived {};
	struct Destroyed {};
	struct RunnerCreated {
		Entity *runner;
		Entity *GetRunner() { return runner; }
	};
	struct RunnerDestroyed {
		Entity* other;
		Entity* GetOther() { return other; }
	};

	struct Render {
		std::vector<Renderer::MeshData> &data;
		mat4 &cameraTransform;
	};

	struct Collided {
		Entity *other;
		Entity *GetOther() { return other; } // for python
	};

//	struct StartGame {};

	struct ScriptEvent {
		python::object pyevent;
	};

	struct TriggerEnter {
		Entity *entity;
		Entity *GetEntity() { return entity; }
	};

	struct TriggerExit {
		Entity *entity;
		Entity *GetEntity() { return entity; }
	};
};

// Expose the template class with all event types as EventSystem (so we don't have to
// write EventSystem<Event1, Event2, ...> everywhere).
//
// To add an event type, add it to the template arguments here.
using EventSystem = EventSystem_<
	Events::Sound,
	Events::Render,
	Events::Collided, 
	Events::Destroyed, 
	Events::Infected,
	Events::RunnerCreated,
//	Events::StartGame,
	Events::ScriptEvent,
	Events::TriggerEnter,
	Events::TriggerExit,
	Events::RunnerDestroyed,
	Events::Revived
>;