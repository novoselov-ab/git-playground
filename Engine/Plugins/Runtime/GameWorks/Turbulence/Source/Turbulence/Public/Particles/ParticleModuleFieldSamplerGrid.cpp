#include "TurbulencePCH.h"

/*------------------------------------------------------------------------------
Grid
------------------------------------------------------------------------------*/
UParticleModuleFieldSamplerGrid::UParticleModuleFieldSamplerGrid(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UParticleModuleFieldSamplerGrid::CompileModule(FParticleEmitterBuildInfo& EmitterInfo)
{
#if WITH_APEX_TURBULENCE
	FGPUSpriteLocalFieldSamplerInfo LocalFieldSampler;
	LocalFieldSampler.FieldSamplerAsset = GridAsset;
	FTransform LocalTransform;
	LocalTransform.SetTranslation(RelativeTranslation);
	LocalTransform.SetRotation(RelativeRotation.Quaternion());
	LocalFieldSampler.Transform = LocalTransform;
	EmitterInfo.LocalFieldSamplers.Add(LocalFieldSampler);
#endif
}


