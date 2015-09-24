#pragma once

#include "ParticleEmitterInstances.h"

struct FFlexParticleEmitterInstance : public FParticleEmitterInstance
{
	FFlexParticleEmitterInstance();
	virtual ~FFlexParticleEmitterInstance();

	UParticleModuleTypeDataFlex* FlexTypeData;

	virtual void InitParameters(UParticleEmitter* InTemplate, UParticleSystemComponent* InComponent, bool bClearResources = true) override;
	virtual void Init() override;

};