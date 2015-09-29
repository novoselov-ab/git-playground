#include "FlexPCH.h"

UParticleModuleTypeDataFlex::UParticleModuleTypeDataFlex(const FObjectInitializer& ObjectInitializer):
	UParticleModuleTypeDataBase(ObjectInitializer),
	FlexContainerTemplate(nullptr),
	Phase(),
	Mass(0)
{

}

UParticleModuleTypeDataFlex::~UParticleModuleTypeDataFlex()
{

}

FParticleEmitterInstance* UParticleModuleTypeDataFlex::CreateInstance(UParticleEmitter* InEmitterParent, UParticleSystemComponent* InComponent)
{
	return nullptr;
}

