#pragma once

#include <PhysX/PxPhysicsAPI.h>
#include <PhysX/geometry/PxGeometry.h>

#include "Component.h"
#include "Physics.h"

class RigidBody : public Component
{
public:
	RigidBody(Physics &physics, PxMaterial &material, PxGeometry &geometry, PxTransform &transform, bool dynamic = true);
	~RigidBody();

	void RegisterHandlers() override;

private:
	PxRigidActor *body_;
};

