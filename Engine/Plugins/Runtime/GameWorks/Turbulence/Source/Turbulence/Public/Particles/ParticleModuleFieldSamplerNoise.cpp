#include "TurbulencePCH.h"

/*------------------------------------------------------------------------------
Noise
------------------------------------------------------------------------------*/
UParticleModuleFieldSamplerNoise::UParticleModuleFieldSamplerNoise(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UParticleModuleFieldSamplerNoise::CompileModule(FParticleEmitterBuildInfo& EmitterInfo)
{
#if WITH_APEX_TURBULENCE
	FGPUSpriteLocalFieldSamplerInfo LocalFieldSampler;
	LocalFieldSampler.FieldSamplerAsset = NoiseAsset;
	FTransform LocalTransform;
	LocalTransform.SetTranslation(RelativeTranslation);
	LocalTransform.SetRotation(RelativeRotation.Quaternion());
	LocalFieldSampler.Transform = LocalTransform;
	EmitterInfo.LocalFieldSamplers.Add(LocalFieldSampler);
#endif
}
