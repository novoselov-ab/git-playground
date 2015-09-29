#pragma once

#include "ParticleEmitterInstances.h"

struct FFlexParticleEmitterInstance : public FParticleEmitterInstance, public IFlexContainerClient
{
	FFlexParticleEmitterInstance();
	virtual ~FFlexParticleEmitterInstance();

	UParticleModuleTypeDataFlex* FlexTypeData;

	FFlexContainerInstance* Container;
	int32 Phase;

	/** The offset to the index of the associated flex particle			*/
	int32 FlexDataOffset;
	/** Set if anisotropy data is available for rendering				*/
	uint32 bFlexAnisotropyData : 1;


	virtual void Init() override;

// IFlexContainerClient interface
	virtual bool IsEnabled() override;
	virtual FBoxSphereBounds GetBounds() override;
// IFlextContainerClient

};
