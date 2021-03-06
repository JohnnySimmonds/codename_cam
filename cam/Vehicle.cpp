#include <memory>
#include <iostream>

#include <PhysX/PxPhysicsAPI.h>
#include <PhysX/PxBatchQuery.h>
#include <PhysX/PxBatchQueryDesc.h>
#include <PhysX/vehicle/PxVehicleSDK.h>
#include <PhysX/vehicle/PxVehicleUtil.h>
#include <PhysX/PxScene.h>
#include "windows.h"
#include "Xinput.h"

#include "Vehicle.h"
#include "Controller.h"
#include "Entity.h"

using namespace physx;

static PxQueryHitType::Enum WheelPreFilterShader(PxFilterData /* unused */,
												 PxFilterData data,
												 const void * /* unused */,
												 PxU32 /* unused */,
												 PxHitFlags &flags)
{
	return (data.word3 & Physics::DRIVABLE_SURFACE) ? PxQueryHitType::eBLOCK : PxQueryHitType::eNONE;
}

static std::vector<PxVec3> ComputeWheelOffsets(const Vehicle::Configuration &config)
{
	const PxF32 wheelFrontZ = config.chassisDimensions.z * 0.3f;
	const PxF32 wheelRearZ = -config.chassisDimensions.z * 0.3f;
	const PxVec3 &dimensions = config.chassisDimensions;
	const PxF32 wheelWidth = config.wheelWidth;
	const PxF32 wheelRadius = config.wheelRadius;
	const PxU32 nWheels = config.nWheels;

	std::vector<PxVec3> ret(nWheels);

	const PxF32 numLeftWheels = nWheels / 2.0f;
	const PxF32 deltaZ = (wheelFrontZ - wheelRearZ) / (numLeftWheels - 1.0f);

	//Set the outside of the left and right wheels to be flush with the chassis.
	//Set the top of the wheel to be just touching the underside of the chassis.
	//Begin by setting the rear-left/rear-right/front-left,front-right wheels.
	ret[PxVehicleDrive4WWheelOrder::eREAR_LEFT] = PxVec3((-dimensions.x + wheelWidth)*0.5f, -(dimensions.y / 2 + wheelRadius), wheelRearZ + 0 * deltaZ*0.5f);
	ret[PxVehicleDrive4WWheelOrder::eREAR_RIGHT] = PxVec3((+dimensions.x - wheelWidth)*0.5f, -(dimensions.y / 2 + wheelRadius), wheelRearZ + 0 * deltaZ*0.5f);
	ret[PxVehicleDrive4WWheelOrder::eFRONT_LEFT] = PxVec3((-dimensions.x + wheelWidth)*0.5f, -(dimensions.y / 2 + wheelRadius), wheelRearZ + (numLeftWheels - 1)*deltaZ);
	ret[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT] = PxVec3((+dimensions.x - wheelWidth)*0.5f, -(dimensions.y / 2 + wheelRadius), wheelRearZ + (numLeftWheels - 1)*deltaZ);
	//Set the remaining wheels.
	for (PxU32 i = 2, wheelCount = 4; i < nWheels - 2; i += 2, wheelCount += 2)
	{
		ret[wheelCount + 0] = PxVec3((-dimensions.x + wheelWidth)*0.5f, -(dimensions.y / 2 + wheelRadius), wheelRearZ + i*deltaZ*0.5f);
		ret[wheelCount + 1] = PxVec3((+dimensions.x - wheelWidth)*0.5f, -(dimensions.y / 2 + wheelRadius), wheelRearZ + i*deltaZ*0.5f);
	}

	return ret;
}

static PxVehicleWheelsSimData *SetupWheels(Vehicle::Configuration &config)
{
	config.wheelOffsets = ComputeWheelOffsets(config);

	PxVehicleWheelsSimData *ret = PxVehicleWheelsSimData::allocate(config.nWheels);
	//ret->setChassisMass(config.chassisMass)

	std::vector<PxVehicleWheelData> wheels(config.nWheels);
	for (auto &wheel : wheels) {
		wheel.mMass = config.wheelMass;
		wheel.mMOI = config.wheelMOI;
		wheel.mRadius = config.wheelRadius;
		wheel.mWidth = config.wheelWidth;

		wheels[PxVehicleDrive4WWheelOrder::eREAR_LEFT].mMaxHandBrakeTorque = config.handbrakeTorque;
		wheels[PxVehicleDrive4WWheelOrder::eREAR_RIGHT].mMaxHandBrakeTorque = config.handbrakeTorque;

		wheels[PxVehicleDrive4WWheelOrder::eFRONT_LEFT].mMaxSteer = config.steerAngle;
		wheels[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT].mMaxSteer = config.steerAngle;
	}

	std::vector<PxVehicleTireData> tires(config.nWheels);
	for (auto &tire : tires) {
		tire.mType = 0;
	}

	std::vector<PxVehicleSuspensionData> suspensions(config.nWheels);
	std::vector<PxF32> sprungMasses(config.nWheels);

	PxVehicleComputeSprungMasses(config.nWheels, config.wheelOffsets.data(), config.chassisOffset, config.chassisMass, 1, sprungMasses.data());
	for (size_t i = 0; i < config.nWheels; ++i) {
		suspensions[i].mMaxCompression = config.maxCompression;
		suspensions[i].mMaxDroop = config.maxDroop;
		suspensions[i].mSpringStrength = config.suspensionStrength;
		suspensions[i].mSpringDamperRate = config.suspensionDamping;
		suspensions[i].mSprungMass = sprungMasses[i];
	}

	for (size_t i = 0; i < config.nWheels; i += 2) {
		suspensions[i].mCamberAtRest = config.camberAtRest;
		suspensions[i + 1].mCamberAtRest = -config.camberAtRest;
		suspensions[i].mCamberAtMaxDroop = config.camberAtMaxDroop;
		suspensions[i + 1].mCamberAtMaxDroop = -config.camberAtMaxDroop;
		suspensions[i].mCamberAtMaxCompression = config.camberAtMaxCompression;
		suspensions[i + 1].mCamberAtMaxCompression = -config.camberAtMaxCompression;
	}

	std::vector<PxVec3> travel(config.nWheels, PxVec3(0.0f, -1.0f, 0.0f));
	std::vector<PxVec3> wheelOffsets(config.wheelOffsets);

	PxFilterData queryfilter {0, 0, 0, Physics::NONDRIVABLE_SURFACE};

	for (PxU32 i = 0; i < config.nWheels; ++i) {
		wheelOffsets[i] -= config.chassisOffset;

		ret->setWheelData(i, wheels[i]);
		ret->setTireData(i, tires[i]);
		ret->setSuspensionData(i, suspensions[i]);
		ret->setSuspTravelDirection(i, travel[i]);
		ret->setWheelCentreOffset(i, wheelOffsets[i]);
		ret->setSuspForceAppPointOffset(i, wheelOffsets[i]);
		ret->setTireForceAppPointOffset(i, wheelOffsets[i]);
		ret->setSceneQueryFilterData(i, queryfilter);
	}
	
	return ret;
}

static PxVehicleDriveSimData4W SetupDrive(const Vehicle::Configuration &config)
{
	// TODO can we disable this stuff?...
	PxVehicleDriveSimData4W ret;
	PxVehicleDifferential4WData diff;
	diff.mType = PxVehicleDifferential4WData::eDIFF_TYPE_LS_4WD;
	ret.setDiffData(diff);

	PxVehicleEngineData engine;
	engine.mMOI = config.maxSpeed;
	engine.mPeakTorque = config.torque;
	engine.mMaxOmega = config.maxOmega;
	ret.setEngineData(engine);

	PxVehicleGearsData gears;
	gears.mSwitchTime = 0.2f;
	ret.setGearsData(gears);

	PxVehicleClutchData clutch;
	clutch.mStrength = 10.0f;
	ret.setClutchData(clutch);

	// lol wut physx
	PxVehicleAckermannGeometryData ackermann;
	ackermann.mAccuracy = 1.0f;
	ackermann.mAxleSeparation =
		config.wheelOffsets[PxVehicleDrive4WWheelOrder::eFRONT_LEFT].z -
		config.wheelOffsets[PxVehicleDrive4WWheelOrder::eREAR_LEFT].z;
	ackermann.mFrontWidth =
		config.wheelOffsets[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT].x -
		config.wheelOffsets[PxVehicleDrive4WWheelOrder::eFRONT_LEFT].x;
	ackermann.mRearWidth =
		config.wheelOffsets[PxVehicleDrive4WWheelOrder::eREAR_RIGHT].x -
		config.wheelOffsets[PxVehicleDrive4WWheelOrder::eREAR_LEFT].x;	
	
	ret.setAckermannGeometryData(ackermann);

	return ret;
}

static PxConvexMesh *CreateConvexMesh(const std::vector<PxVec3> &verts, PxPhysics *physics, PxCooking *cooking)
{
	// Create descriptor for convex mesh
	PxConvexMeshDesc convexDesc;
	convexDesc.points.count = (PxU32)verts.size();
	convexDesc.points.stride = sizeof(PxVec3);
	convexDesc.points.data = verts.data();
	convexDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX | PxConvexFlag::eINFLATE_CONVEX;

	PxConvexMesh* convexMesh = NULL;
	PxDefaultMemoryOutputStream buf;
	if (cooking->cookConvexMesh(convexDesc, buf))
	{
		PxDefaultMemoryInputData id(buf.getData(), buf.getSize());
		convexMesh = physics->createConvexMesh(id);
	}

	return convexMesh;
}

static PxConvexMesh *CreateChassisMesh(const PxVec3 &dimensions, PxPhysics *physics, PxCooking *cooking)
{
	const PxF32 x = dimensions.x * 0.5f;
	const PxF32 y = dimensions.y * 0.5f;
	const PxF32 z = dimensions.z * 0.5f;

	std::vector<PxVec3> verts{
		PxVec3(x,y,-z),
		PxVec3(x,y,z),
		PxVec3(x,-y,z),
		PxVec3(x,-y,-z),
		PxVec3(-x,y,-z),
		PxVec3(-x,y,z),
		PxVec3(-x,-y,z),
		PxVec3(-x,-y,-z)
	};

	return CreateConvexMesh(verts, physics, cooking);
}

static PxConvexMesh *CreateWheelMesh(PxF32 width, PxF32 radius, PxPhysics *physics, PxCooking *cooking)
{
	std::vector<PxVec3> points;
	for (PxU32 i = 0; i < 16; i++)
	{
		const PxF32 cosTheta = PxCos(i*PxPi*2.0f / 16.0f);
		const PxF32 sinTheta = PxSin(i*PxPi*2.0f / 16.0f);
		const PxF32 y = radius*cosTheta;
		const PxF32 z = radius*sinTheta;
		points.push_back(PxVec3(-width / 2.0f, y, z));
		points.push_back(PxVec3(width / 2.0f, y, z));
	}

	return CreateConvexMesh(points, physics, cooking);
}

static PxRigidDynamic *CreateVehicleActor(Vehicle::Configuration config, PxPhysics *physics, PxCooking *cooking)
{
	auto material = physics->createMaterial(0.5f, 0.5f, 0.6f);

	std::vector<PxConvexMesh *> wheelMeshes;
	auto chassisMesh = CreateChassisMesh(config.chassisDimensions, physics, cooking);

	for (size_t i = 0; i < config.nWheels; ++i) {
		wheelMeshes.push_back(CreateWheelMesh(config.wheelWidth, config.wheelRadius, physics, cooking));
	}

	auto ret = physics->createRigidDynamic(PxTransform(config.position, config.rotation));

	PxFilterData wheelSimFilter { Physics::WHEEL, Physics::WHEEL_COLLISIONS, 0, 0 };
	PxFilterData chassisSimFilter { Physics::CHASSIS, Physics::CHASSIS_COLLISIONS, 0, 0 };
	PxFilterData wheelQueryFilter{ 0, 0, 0, Physics::NONDRIVABLE_SURFACE };
	PxFilterData chassisQueryFilter{ 0, 0, 0, Physics::NONDRIVABLE_SURFACE };

	for (PxU32 i = 0; i < wheelMeshes.size(); ++i) {
		PxConvexMeshGeometry g(wheelMeshes[i]);
		auto shape = ret->createShape(g, *material);

		shape->setSimulationFilterData(wheelSimFilter);
		shape->setQueryFilterData(wheelQueryFilter);
		shape->setLocalPose(PxTransform(PxIdentity));

		wheelMeshes[i]->release();
	}

	PxConvexMeshGeometry g(chassisMesh);
	auto shape = ret->createShape(g, *material);

	shape->setSimulationFilterData(chassisSimFilter);
	shape->setQueryFilterData(chassisQueryFilter);
	shape->setLocalPose(PxTransform(PxIdentity));

	chassisMesh->release();

	ret->setMass(config.chassisMass);
	ret->setMassSpaceInertiaTensor(config.chassisMOI);
	ret->setCMassLocalPose(PxTransform(config.chassisOffset, PxQuat(PxIdentity)));

	material->release();

	return ret;
}

static PxBatchQueryDesc CreateQueryDescription(PxRaycastQueryResult *queryBuffer,
											   PxRaycastHit *hitBuffer)
{
	PxBatchQueryDesc ret(6, 0, 0);
	ret.queryMemory.userRaycastResultBuffer = queryBuffer;
	ret.queryMemory.userRaycastTouchBuffer = hitBuffer;
	ret.queryMemory.raycastTouchBufferSize = 6;
	ret.preFilterShader = WheelPreFilterShader;

	return ret;
}

static PxVehicleDrivableSurfaceToTireFrictionPairs *CreateFrictionPairs(PxPhysics *physics, float mass, float friction)
{
	PxVehicleDrivableSurfaceToTireFrictionPairs *ret =
		PxVehicleDrivableSurfaceToTireFrictionPairs::allocate(1, 1);

	// TODO do we ever actually want to change the surface/tire types?
	// This is probably how we can add oil slicks etc.
	const PxMaterial *surfaces[1] = { physics->createMaterial(.5f, .5f, 0.6f) };
	PxVehicleDrivableSurfaceType types[1] = { 0 };
	ret->setup(1, 1, surfaces, types);
	ret->setTypePairFriction(0, 0, friction * mass);

	return ret;
}

void Vehicle::SetFriction(float friction)
{
	if (frictionpairs_) frictionpairs_->release();
	frictionpairs_ = CreateFrictionPairs(physics_.GetPhysics(), mass_, friction);
}

float Vehicle::GetOmega()
{
	return vehicle_->mDriveSimData.getEngineData().mMaxOmega;
}

float Vehicle::GetTorque()
{
	return vehicle_->mDriveSimData.getEngineData().mPeakTorque;
}

void Vehicle::SetOmega(float omega)
{
	auto engine = vehicle_->mDriveSimData.getEngineData();
	engine.mMaxOmega = omega;
	vehicle_->mDriveSimData.setEngineData(engine);
}

void Vehicle::SetTorque(float torque)
{
	auto engine = vehicle_->mDriveSimData.getEngineData();
	engine.mPeakTorque = torque;
	vehicle_->mDriveSimData.setEngineData(engine);
}

Vehicle::Vehicle(Physics &physics, std::shared_ptr<Controller> controller, Configuration &config) :
	physics_(physics),
	controller_(controller),
	querybuffer_(config.nWheels),
	hitbuffer_(config.nWheels),
	active_(true),
	mass_(config.chassisMass)
{
	auto desc = CreateQueryDescription(querybuffer_.data(), hitbuffer_.data());
	batchquery_ = physics.GetScene()->createBatchQuery(desc);

	auto wheels = SetupWheels(config);
	auto drive = SetupDrive(config);

	actor_ = CreateVehicleActor(config, physics.GetPhysics(), physics.GetCooking());
	vehicle_ = PxVehicleDrive4W::allocate(config.nWheels);
	vehicle_->setup(physics.GetPhysics(), actor_, *wheels, drive, config.nWheels - 4);

	physics.GetScene()->addActor(*vehicle_->getRigidDynamicActor());

	vehicle_->setToRestState();
	vehicle_->mDriveDynData.forceGearChange(PxVehicleGearsData::eFIRST);

	SetFriction(1.0f);

	wheels->free();

	alGenSources(1, &source); //sound stuff
	
	if (controller->getPlayer() == 0)
		player = true;

	physics_.RegisterVehicle(this);
}

Vehicle::Vehicle(Physics &physics, std::shared_ptr<aiController> aicontroller, Configuration &config) :
	Vehicle(physics, std::shared_ptr<Controller>(aicontroller), config)
{
	player = false;
}

Vehicle::~Vehicle()
{
	std::cout << "killing a vehicle!\n";
	physics_.UnregisterVehicle(this);
	physics_.GetScene()->removeActor(*vehicle_->getRigidDynamicActor());
	vehicle_->getRigidDynamicActor()->release();
	vehicle_->free();
	frictionpairs_->release();
	batchquery_->release();
}


void Vehicle::Drive()
{
	if (!active_) return;
	
	controller_->UpdateState();
	prevChoice = soundChoice;
	soundPosition = entity_->GetTransform().position;
	switch (controller_->getAcceleration())
	{
	case C_FAST:
		vehicle_->mDriveDynData.setCurrentGear(PxVehicleGearsData::eFIRST);
		input_.setAnalogAccel(1.0f);
		input_.setAnalogBrake(0.0f);
		soundChoice = 2;
		break;
	case C_NEUTRAL:
		input_.setAnalogAccel(0.0f);
		soundChoice = 1;
		break;
	case C_REVERSE:
		soundChoice = 3;
		vehicle_->mDriveDynData.setCurrentGear(PxVehicleGearsData::eREVERSE);
		input_.setAnalogAccel(0.5f);
		break;
	}

	

	switch (controller_->getBrake()) {
	case true:
		input_.setAnalogBrake(1.0f);
		break;
	case false:
		input_.setAnalogBrake(0.0f);
		break;
	}
	if (prevSoundChoice != soundChoice)
		changeSound = true;
	input_.setAnalogSteer(controller_->getDirectional());
}

void Vehicle::Step(float dt)
{

	PxVehicleWheels *vehicles[1] = { vehicle_ };

	PxVehicleSuspensionRaycasts(
		batchquery_,
		1,
		vehicles,
		(PxU32)querybuffer_.size(),
		querybuffer_.data()
	);

	PxVehicleUpdates(
		dt,
		physics_.GetScene()->getGravity(),
		*frictionpairs_,
		1,
		vehicles,
		nullptr
	);
}

void Vehicle::Update(seconds dt)
{
	// TODO do we want to configure these per-vehicle?
	static PxF32 steerVsForwardSpeedData[2 * 8] =
	{
		0.0f,		0.75f,
		5.0f,		0.75f,
		30.0f,		0.125f,
		120.0f,		0.1f,
		PX_MAX_F32, PX_MAX_F32,
		PX_MAX_F32, PX_MAX_F32,
		PX_MAX_F32, PX_MAX_F32,
		PX_MAX_F32, PX_MAX_F32
	};

	static PxFixedSizeLookupTable<8> steerVsForwardSpeedTable(steerVsForwardSpeedData, 4);

	static PxVehiclePadSmoothingData padSmoothingData =
	{
		{
			6.0f,	//rise rate eANALOG_INPUT_ACCEL
			6.0f,	//rise rate eANALOG_INPUT_BRAKE		
			6.0f,	//rise rate eANALOG_INPUT_HANDBRAKE	
			3.0f,	//rise rate eANALOG_INPUT_STEER_LEFT
			3.0f,	//rise rate eANALOG_INPUT_STEER_RIGHT
		},
		{
			10.0f,	//fall rate eANALOG_INPUT_ACCEL
			10.0f,	//fall rate eANALOG_INPUT_BRAKE		
			10.0f,	//fall rate eANALOG_INPUT_HANDBRAKE	
			20.0f,	//fall rate eANALOG_INPUT_STEER_LEFT
			20.0f	//fall rate eANALOG_INPUT_STEER_RIGHT
		}
	};

	Drive();

	std::vector<PxWheelQueryResult> wheelresults(vehicle_->mWheelsSimData.getNbWheels());

	PxVehicleWheelQueryResult vehicleresults[1] = {
		{ wheelresults.data(), vehicle_->mWheelsSimData.getNbWheels() }
	};

	PxVehicleDrive4WSmoothAnalogRawInputsAndSetAnalogInputs(
		padSmoothingData,
		steerVsForwardSpeedTable,
		input_,
		dt.count(),
		PxVehicleIsInAir(vehicleresults[0]),
		*vehicle_
	);
}

void Vehicle::getSource(Events::Sound event)
{
	event.sources.push_back(source);
	event.choice.push_back(soundChoice);
	event.pChoice.push_back(prevChoice);
	event.soundPosition.push_back(soundPosition);
	event.isPlayer.push_back(player);
	event.forwardVecs.push_back(entity_->GetTransform().Forward());
}

void Vehicle::RegisterHandlers()
{
	auto t = actor_->getGlobalPose();
	entity_->GetTransform().position = vec3(t.p.x, t.p.y, t.p.z);
	entity_->GetTransform().rotation = quaternion(t.q.w, t.q.x, t.q.y, t.q.z);
	actor_->userData = entity_;

	handler_ = [this](Events::Sound e) {
		getSource(e);
	};

	entity_->RegisterEventHandler(&handler_);
}


