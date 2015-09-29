#include "FlexPCH.h"

FFlexParticleEmitterInstance::FFlexParticleEmitterInstance():
	FParticleEmitterInstance(),
	FlexTypeData(nullptr),
	Container(nullptr),
	Phase(0),
	FlexDataOffset(0),
	bFlexAnisotropyData(false)
{

}

FFlexParticleEmitterInstance::~FFlexParticleEmitterInstance()
{
	if (Container)
	{
		for (int32 i = 0; i < ActiveParticles; i++)
		{
			DECLARE_PARTICLE(Particle, ParticleData + ParticleStride * ParticleIndices[i]);
			verify(FlexDataOffset > 0);
			int32 CurrentOffset = FlexDataOffset;
			const uint8* ParticleBase = (const uint8*)&Particle;
			PARTICLE_ELEMENT(int32, FlexParticleIndex);
			Container->DestroyParticle(FlexParticleIndex);
		}

		Container->Unregister(this);
	}
}

void FFlexParticleEmitterInstance::Init()
{
	FParticleEmitterInstance::Init();

	if (FlexTypeData->FlexContainerTemplate && (!GIsEditor || GIsPlayInEditorWorld))
	{
		// TODO: Get the FFlexContainerInstance from somewhere global. Originally was stored in the FPhysScene, now get it from the actor in the scene

		Container->Register(this);
		Phase = Container->GetPhase(FlexTypeData->Phase);
	}
}

bool FFlexParticleEmitterInstance::IsEnabled()
{
	return Container != nullptr;
}

FBoxSphereBounds FFlexParticleEmitterInstance::GetBounds()
{
	return FBoxSphereBounds(GetBoundingBox());
}

