// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "EnginePrivate.h"
#include "PhysicsPublic.h"
#include "PhysicsEngine/PhysicsSettings.h"

#if WITH_SUBSTEPPING

#if WITH_PHYSX
#include "PhysXSupport.h"
#include "../Vehicles/PhysXVehicleManager.h"
#endif

#include "PhysSubstepTasks.h"

#if WITH_PHYSX
FPhysSubstepTask::FPhysSubstepTask(PxApexScene * GivenScene) :
	NumSubsteps(0),
	SubTime(0.f),
	DeltaSeconds(0.f),
	External(0),
	FullSimulationTask(0),
	Alpha(0.f),
	StepScale(0.f),
	TotalSubTime(0.f),
	CurrentSubStep(0),
	VehicleManager(NULL),
	PAScene(GivenScene)
{
	check(PAScene);
}
#endif

void FPhysSubstepTask::SwapBuffers()
{
	External = !External;
}

void FPhysSubstepTask::RemoveBodyInstance(FBodyInstance* BodyInstance)
{
	
	PhysTargetBuffers[External].Remove(BodyInstance);
	
	//This function gets called when we terminate a body instance. (FBodyInstance::TermBody)
	//To do that FBodyInstance needs to be holding on to the scene write lock.
	//This is the same lock that we use when iterating over the internal buffer - so it's safe to modify this buffer from this thread.
	//If this assumption changes you need to take care of accessing the buffer from both threads
	PhysTargetBuffers[!External].Remove(BodyInstance);
}

void FPhysSubstepTask::SetKinematicTarget(FBodyInstance* Body, const FTransform& TM)
{
#if WITH_PHYSX
	check(Body);
	TM.DiagnosticCheckNaN_All();

	PxRigidDynamic * PRigidDynamic = Body->GetPxRigidDynamic();
	SCOPED_SCENE_READ_LOCK(PRigidDynamic->getScene());
	//We only interpolate kinematic actors
	if (!IsRigidBodyNonKinematic(PRigidDynamic))
	{
		FKinematicTarget KinmaticTarget(Body, TM);
		FPhysTarget & TargetState = PhysTargetBuffers[External].FindOrAdd(Body);
		TargetState.bKinematicTarget = true;
		TargetState.KinematicTarget = KinmaticTarget;
	}
#endif
}

bool FPhysSubstepTask::GetKinematicTarget(const FBodyInstance* Body, FTransform& OutTM) const
{
#if WITH_PHYSX
	const PxRigidDynamic * PRigidDynamic = Body->GetPxRigidDynamic();
	SCOPED_SCENE_READ_LOCK(PRigidDynamic->getScene());
	if (const FPhysTarget* TargetState = PhysTargetBuffers[External].Find(Body))
	{
		if (TargetState->bKinematicTarget)
		{
			OutTM = TargetState->KinematicTarget.TargetTM;
			return true;
		}
	}
#endif

	return false;
}

void FPhysSubstepTask::AddCustomPhysics(FBodyInstance* Body, const FCalculateCustomPhysics& CalculateCustomPhysics)
{
#if WITH_PHYSX
	check(Body);
	PxRigidBody* PRigidBody = Body->GetPxRigidBody();
	SCOPED_SCENE_READ_LOCK(PRigidBody->getScene());
	//Limit custom physics to non kinematic actors
	if (IsRigidBodyNonKinematic(PRigidBody))
	{
		FCustomTarget CustomTarget(CalculateCustomPhysics);

		FPhysTarget & TargetState = PhysTargetBuffers[External].FindOrAdd(Body);
		TargetState.CustomPhysics.Add(CustomTarget);
	}
#endif
}

void FPhysSubstepTask::AddForce(FBodyInstance* Body, const FVector& Force, bool bAccelChange)
{
#if WITH_PHYSX
	check(Body);
	PxRigidBody* PRigidBody = Body->GetPxRigidBody();
	SCOPED_SCENE_READ_LOCK(PRigidBody->getScene());
	//We should only apply forces on non kinematic actors
	if (IsRigidBodyNonKinematic(PRigidBody))
	{

		FForceTarget ForceTarget;
		ForceTarget.bPosition = false;
		ForceTarget.Force = Force;
		ForceTarget.bAccelChange = bAccelChange;

		FPhysTarget & TargetState = PhysTargetBuffers[External].FindOrAdd(Body);
		TargetState.Forces.Add(ForceTarget);
	}
#endif
}

void FPhysSubstepTask::AddForceAtPosition(FBodyInstance* Body, const FVector& Force, const FVector& Position)
{
#if WITH_PHYSX
	check(Body);

	PxRigidBody* PRigidBody = Body->GetPxRigidBody();
	SCOPED_SCENE_READ_LOCK(PRigidBody->getScene());
	//We should only apply forces on non kinematic actors
	if (IsRigidBodyNonKinematic(PRigidBody))
	{
		FForceTarget ForceTarget;
		ForceTarget.bPosition = true;
		ForceTarget.Force = Force;
		ForceTarget.Position = Position;

		FPhysTarget & TargetState = PhysTargetBuffers[External].FindOrAdd(Body);
		TargetState.Forces.Add(ForceTarget);
	}
#endif
}
void FPhysSubstepTask::AddTorque(FBodyInstance* Body, const FVector& Torque, bool bAccelChange)
{
#if WITH_PHYSX
	check(Body);

	PxRigidBody* PRigidBody = Body->GetPxRigidBody();
	SCOPED_SCENE_READ_LOCK(PRigidBody->getScene());
	//We should only apply torque on non kinematic actors
	if (IsRigidBodyNonKinematic(PRigidBody))
	{
		FTorqueTarget TorqueTarget;
		TorqueTarget.Torque = Torque;
		TorqueTarget.bAccelChange = bAccelChange;

		FPhysTarget & TargetState = PhysTargetBuffers[External].FindOrAdd(Body);
		TargetState.Torques.Add(TorqueTarget);
	}
#endif
}

void FPhysSubstepTask::AddRadialForceToBody(FBodyInstance* Body, const FVector& Origin, const float Radius, const float Strength, const uint8 Falloff, const bool bAccelChange)
{
#if WITH_PHYSX
	check(Body);

	PxRigidBody* PRigidBody = Body->GetPxRigidBody();
	SCOPED_SCENE_READ_LOCK(PRigidBody->getScene());
	//We should only apply torque on non kinematic actors
	if (IsRigidBodyNonKinematic(PRigidBody))
	{
		FRadialForceTarget RadialForceTarget;
		RadialForceTarget.Origin = Origin;
		RadialForceTarget.Radius = Radius;
		RadialForceTarget.Strength = Strength;
		RadialForceTarget.Falloff = Falloff;
		RadialForceTarget.bAccelChange = bAccelChange;

		FPhysTarget & TargetState = PhysTargetBuffers[External].FindOrAdd(Body);
		TargetState.RadialForces.Add(RadialForceTarget);
	}
#endif
}

/** Applies custom physics - Assumes caller has obtained writer lock */
void FPhysSubstepTask::ApplyCustomPhysics(const FPhysTarget& PhysTarget, FBodyInstance* BodyInstance, float DeltaTime)
{
#if WITH_PHYSX
	for (int32 i = 0; i < PhysTarget.CustomPhysics.Num(); ++i)
	{
		const FCustomTarget& CustomTarget = PhysTarget.CustomPhysics[i];

		CustomTarget.CalculateCustomPhysics->ExecuteIfBound(DeltaTime, BodyInstance);
	}
#endif
}

/** Applies forces - Assumes caller has obtained writer lock */
void FPhysSubstepTask::ApplyForces(const FPhysTarget& PhysTarget, FBodyInstance* BodyInstance)
{
#if WITH_PHYSX
	/** Apply Forces */
	PxRigidBody* PRigidBody = BodyInstance->GetPxRigidBody();

	for (int32 i = 0; i < PhysTarget.Forces.Num(); ++i)
	{
		const FForceTarget& ForceTarget = PhysTarget.Forces[i];

		if (ForceTarget.bPosition)
		{
			PxRigidBodyExt::addForceAtPos(*PRigidBody, U2PVector(ForceTarget.Force), U2PVector(ForceTarget.Position), PxForceMode::eFORCE, true);
		}
		else
		{
			PRigidBody->addForce(U2PVector(ForceTarget.Force), ForceTarget.bAccelChange ? PxForceMode::eACCELERATION : PxForceMode::eFORCE, true);
		}
	}
#endif
}

/** Applies torques - Assumes caller has obtained writer lock */
void FPhysSubstepTask::ApplyTorques(const FPhysTarget& PhysTarget, FBodyInstance* BodyInstance)
{
#if WITH_PHYSX
	/** Apply Torques */
	PxRigidBody* PRigidBody = BodyInstance->GetPxRigidBody();

	for (int32 i = 0; i < PhysTarget.Torques.Num(); ++i)
	{
		const FTorqueTarget& TorqueTarget = PhysTarget.Torques[i];
		PRigidBody->addTorque(U2PVector(TorqueTarget.Torque), TorqueTarget.bAccelChange ? PxForceMode::eACCELERATION : PxForceMode::eFORCE, true);
	}
#endif
}

/** Applies radial forces - Assumes caller has obtained writer lock */
void FPhysSubstepTask::ApplyRadialForces(const FPhysTarget& PhysTarget, FBodyInstance* BodyInstance)
{
#if WITH_PHYSX
	/** Apply Torques */
	PxRigidBody* PRigidBody = BodyInstance->GetPxRigidBody();

	for (int32 i = 0; i < PhysTarget.RadialForces.Num(); ++i)
	{
		const FRadialForceTarget& RadialForceTArget= PhysTarget.RadialForces[i];
		AddRadialForceToPxRigidBody(*PRigidBody, RadialForceTArget.Origin, RadialForceTArget.Radius, RadialForceTArget.Strength, RadialForceTArget.Falloff, RadialForceTArget.bAccelChange);
	}
#endif
}


/** Interpolates kinematic actor transform - Assumes caller has obtained writer lock */
void FPhysSubstepTask::InterpolateKinematicActor(const FPhysTarget& PhysTarget, FBodyInstance* BodyInstance, float InAlpha)
{
#if WITH_PHYSX
	PxRigidDynamic * PRigidDynamic = BodyInstance->GetPxRigidDynamic();
	InAlpha = FMath::Clamp(InAlpha, 0.f, 1.f);

	/** Interpolate kinematic actors */
	if (PhysTarget.bKinematicTarget)
	{
		//It's possible that the actor is no longer kinematic and is now simulating. In that case do nothing
		if (!IsRigidBodyNonKinematic(PRigidDynamic))
		{
			const FKinematicTarget& KinematicTarget = PhysTarget.KinematicTarget;
			const FTransform& TargetTM = KinematicTarget.TargetTM;
			const FTransform& StartTM = KinematicTarget.OriginalTM;
			FTransform InterTM = FTransform::Identity;

			InterTM.SetLocation(FMath::Lerp(StartTM.GetLocation(), TargetTM.GetLocation(), InAlpha));
			InterTM.SetRotation(FMath::Lerp(StartTM.GetRotation(), TargetTM.GetRotation(), InAlpha));

			const PxTransform PNewPose = U2PTransform(InterTM);
			check(PNewPose.isValid());
			PRigidDynamic->setKinematicTarget(PNewPose);
		}
	}
#endif
}

void FPhysSubstepTask::SubstepInterpolation(float InAlpha, float DeltaTime)
{
#if WITH_PHYSX
#if WITH_APEX
	PxScene * PScene = PAScene->getPhysXScene();
#else
	PxScene * PScene = PAScene;
#endif

	PhysTargetMap & Targets = PhysTargetBuffers[!External];

	/** Note: We lock the entire scene before iterating. The assumption is that removing an FBodyInstance from the map will also be wrapped by this lock */
	SCENE_LOCK_WRITE(PScene);

	for (PhysTargetMap::TIterator Itr = Targets.CreateIterator(); Itr; ++Itr)
	{
		FPhysTarget & PhysTarget = Itr.Value();
		FBodyInstance * BodyInstance = Itr.Key();
		PxRigidBody* PRigidBody = BodyInstance->GetPxRigidBody();

		if (PRigidBody == NULL)
		{
			continue;
		}

		//We should only be iterating over actors that belong to this scene
		check(PRigidBody->getScene() == PScene);

		ApplyCustomPhysics(PhysTarget, BodyInstance, DeltaTime);
		ApplyForces(PhysTarget, BodyInstance);
		ApplyTorques(PhysTarget, BodyInstance);
		ApplyRadialForces(PhysTarget, BodyInstance);
		InterpolateKinematicActor(PhysTarget, BodyInstance, InAlpha);
	}

	/** Final substep */
	if (InAlpha >= 1.f)
	{
		Targets.Empty(Targets.Num());
	}

	SCENE_UNLOCK_WRITE(PScene);
#endif
}

float FPhysSubstepTask::UpdateTime(float UseDelta)
{
	float FrameRate = 1.f;
	uint32 MaxSubSteps = 1;

	UPhysicsSettings * PhysSetting = UPhysicsSettings::Get();
	FrameRate = PhysSetting->MaxSubstepDeltaTime;
	MaxSubSteps = PhysSetting->MaxSubsteps;
	
	float FrameRateInv = 1.f / FrameRate;

	//Figure out how big dt to make for desired framerate
	DeltaSeconds = FMath::Min(UseDelta, MaxSubSteps * FrameRate);
	NumSubsteps = FMath::CeilToInt(DeltaSeconds * FrameRateInv);
	NumSubsteps = FMath::Max(NumSubsteps > MaxSubSteps ? MaxSubSteps : NumSubsteps, (uint32) 1);
	SubTime = DeltaSeconds / NumSubsteps;

	return SubTime;
}

#if WITH_PHYSX
void FPhysSubstepTask::StepSimulation(PhysXCompletionTask * Task)
{
	check(SubTime > 0.f);
	check(DeltaSeconds > 0.f);

	FullSimulationTask = Task;
	Alpha = 0.f;
	StepScale = SubTime / DeltaSeconds;
	TotalSubTime = 0.f;
	CurrentSubStep = 0;

	SubstepSimulationStart();
}
#endif

DEFINE_STAT(STAT_SubstepSimulationStart);
DEFINE_STAT(STAT_SubstepSimulationEnd);

void FPhysSubstepTask::SubstepSimulationStart()
{
	SCOPE_CYCLE_COUNTER(STAT_TotalPhysicsTime);
	SCOPE_CYCLE_COUNTER(STAT_SubstepSimulationStart);
#if WITH_PHYSX
	check(SubTime > 0.f);
	check(DeltaSeconds > 0.f);
	
	check(!CompletionEvent.GetReference());	//should be done
	CompletionEvent = FGraphEvent::CreateGraphEvent();
	PhysXCompletionTask* SubstepTask = new PhysXCompletionTask(CompletionEvent, PAScene->getTaskManager());
	ENamedThreads::Type NamedThread = PhysSingleThreadedMode() ? ENamedThreads::GameThread : ENamedThreads::AnyThread;

	DECLARE_CYCLE_STAT(TEXT("FDelegateGraphTask.ProcessPhysSubstepSimulation"),
		STAT_FDelegateGraphTask_ProcessPhysSubstepSimulation,
		STATGROUP_TaskGraphTasks);

	FDelegateGraphTask::CreateAndDispatchWhenReady(
		FDelegateGraphTask::FDelegate::CreateRaw(this, &FPhysSubstepTask::SubstepSimulationEnd),
		GET_STATID(STAT_FDelegateGraphTask_ProcessPhysSubstepSimulation), CompletionEvent, NamedThread, NamedThread);

	++CurrentSubStep;	

	bool bLastSubstep = CurrentSubStep >= NumSubsteps;

	if (!bLastSubstep)
	{

		Alpha += StepScale;
		TotalSubTime += SubTime;
	}

	float DeltaTime = bLastSubstep ? (DeltaSeconds - TotalSubTime) : SubTime;
	float Interpolation = bLastSubstep ? 1.f : Alpha;

#if WITH_VEHICLE
	if (VehicleManager)
	{
		VehicleManager->Update(DeltaTime);
	}
#endif

	SubstepInterpolation(Interpolation, DeltaTime);

#if WITH_APEX
	PAScene->simulate(DeltaTime, bLastSubstep, SubstepTask);
#else
	PAScene->lockWrite();
	PAScene->simulate(DeltaTime, SubstepTask);
	PAScene->unlockWrite();
#endif
	SubstepTask->removeReference();
#endif
}

void FPhysSubstepTask::SubstepSimulationEnd(ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent)
{
#if WITH_PHYSX
	CompletionEvent = NULL;
	if (CurrentSubStep < NumSubsteps)
	{
		uint32 OutErrorCode = 0;
		{
			SCOPE_CYCLE_COUNTER(STAT_TotalPhysicsTime);
			SCOPE_CYCLE_COUNTER(STAT_SubstepSimulationEnd);

#if WITH_APEX
			PAScene->fetchResults(true, &OutErrorCode);
#else
			PAScene->lockWrite();
			PAScene->fetchResults(true, &OutErrorCode);
			PAScene->unlockWrite();
#endif

		}

		if (OutErrorCode != 0)
		{
			UE_LOG(LogPhysics, Log, TEXT("PHYSX FETCHRESULTS ERROR: %d"), OutErrorCode);
		}

		SubstepSimulationStart();
	}
	else
	{
		SCOPE_CYCLE_COUNTER(STAT_TotalPhysicsTime);
		SCOPE_CYCLE_COUNTER(STAT_SubstepSimulationEnd);

		//final step we call fetch on in game thread
		FullSimulationTask->removeReference();
	}
#endif
}


void FPhysSubstepTask::SetVehicleManager(FPhysXVehicleManager * InVehicleManager)
{
	VehicleManager = InVehicleManager;
}

#endif //if WITH_SUBSTEPPING