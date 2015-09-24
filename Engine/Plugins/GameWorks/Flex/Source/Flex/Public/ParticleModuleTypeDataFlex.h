#pragma once
#include "Particles/TypeData/ParticleModuleTypeDataBase.h"

UCLASS(MinimalAPI, editinlinenew, hidecategories = Object, meta = (DisplayName = "AnimTrail Data"))
class UParticleModuleTypeDataFlex : public UParticleModuleTypeDataBase
{
	GENERATED_BODY()
public:

	UParticleModuleTypeDataFlex(const FObjectInitializer& ObjectInitializer);
	virtual ~UParticleModuleTypeDataFlex();

	/** The Flex container to emit into */
	UPROPERTY(EditAnywhere, Category = Flex)
	class UFlexContainer* FlexContainerTemplate;

	/** Phase assigned to spawned Flex particles */
	UPROPERTY(EditAnywhere, Category = Flex)
	FFlexPhase Phase;

	/** Mass assigned to Flex particles */
	UPROPERTY(EditAnywhere, Category = Flex)
	float Mass;


	virtual FParticleEmitterInstance* CreateInstance(UParticleEmitter* InEmitterParent, UParticleSystemComponent* InComponent) override;

};

