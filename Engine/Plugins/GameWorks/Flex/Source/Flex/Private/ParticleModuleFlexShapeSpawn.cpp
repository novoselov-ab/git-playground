#include "FlexPCH.h"
#include "ParticleEmitterInstances.h"

/*-----------------------------------------------------------------------------
UParticleModuleFlexShapeSpawn implementation.
-----------------------------------------------------------------------------*/

UParticleModuleFlexShapeSpawn::UParticleModuleFlexShapeSpawn(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	FlexAsset(nullptr),
	Velocity(0)
{
	bSpawnModule = true;
	bSupported3DDrawMode = false;
}

bool UParticleModuleFlexShapeSpawn::GetSpawnAmount(FParticleEmitterInstance* Owner, int32 Offset, float OldLeftover, float DeltaTime, int32& Number, float& Rate)
{
	return false;
}

bool UParticleModuleFlexShapeSpawn::GetBurstCount(FParticleEmitterInstance* Owner, int32 Offset, float OldLeftover, float DeltaTime, int32& Number)
{
	if (FlexAsset)
	{
		Number = FlexAsset->Particles.Num();
		return true;
	}

	return false;
}

void UParticleModuleFlexShapeSpawn::Update(FParticleEmitterInstance* Owner, int32 Offset, float DeltaTime)
{
}

void UParticleModuleFlexShapeSpawn::Spawn(FParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, FBaseParticle* ParticleBase)
{
	SPAWN_INIT;

	if (FlexAsset)
	{
		// calculate spawn position based on the particle counter
		int32 ParticlesPerShape = FlexAsset->Particles.Num();
		int32 ParticleShapeIndex = Owner->ParticleCounter%ParticlesPerShape;

		FVector Vel = Owner->EmitterToSimulation.TransformVector(FVector(0.0f, 0.0f, Velocity));

		Particle.Location += Owner->EmitterToSimulation.TransformVector(FVector(FlexAsset->Particles[ParticleShapeIndex]));
		Particle.Velocity += Vel;
		Particle.BaseVelocity += Vel;
	}
}

int32 UParticleModuleFlexShapeSpawn::GetMaximumBurstCount()
{
	if (FlexAsset)
		return FlexAsset->Particles.Num();
	else
		return 0;
}
