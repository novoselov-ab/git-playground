#include "TurbulencePCH.h"

/*------------------------------------------------------------------------------
Jet
------------------------------------------------------------------------------*/
UParticleModuleFieldSamplerJet::UParticleModuleFieldSamplerJet(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UParticleModuleFieldSamplerJet::CompileModule(FParticleEmitterBuildInfo& EmitterInfo)
{
#if WITH_APEX_TURBULENCE
	FGPUSpriteLocalFieldSamplerInfo LocalFieldSampler;
	LocalFieldSampler.FieldSamplerAsset = JetAsset;
	FTransform LocalTransform;
	LocalTransform.SetTranslation(RelativeTranslation);
	LocalTransform.SetRotation(RelativeRotation.Quaternion());
	LocalFieldSampler.Transform = LocalTransform;
	EmitterInfo.LocalFieldSamplers.Add(LocalFieldSampler);
#endif
}
