#include "TurbulencePCH.h"



/*------------------------------------------------------------------------------
Attractor
------------------------------------------------------------------------------*/
UParticleModuleFieldSamplerAttractor::UParticleModuleFieldSamplerAttractor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UParticleModuleFieldSamplerAttractor::CompileModule(FParticleEmitterBuildInfo& EmitterInfo)
{
#if WITH_APEX_TURBULENCE
	FGPUSpriteLocalFieldSamplerInfo LocalFieldSampler;
	LocalFieldSampler.FieldSamplerAsset = AttractorAsset;
	FTransform LocalTransform;
	LocalTransform.SetTranslation(RelativeTranslation);
	LocalTransform.SetRotation(RelativeRotation.Quaternion());
	LocalFieldSampler.Transform = LocalTransform;
	EmitterInfo.LocalFieldSamplers.Add(LocalFieldSampler);
#endif
}
