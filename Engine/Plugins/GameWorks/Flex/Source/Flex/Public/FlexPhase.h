#pragma once

#include "FlexPhase.generated.h"

/** Defines flags that control how the particle behaves */
USTRUCT()
struct FFlexPhase
{
	GENERATED_USTRUCT_BODY()

	FFlexPhase();

	/** If true, then particles will be auto-assigned a new group, by default particles will only collide with particles in different groups */
	UPROPERTY(EditAnywhere, Category = Phase)
	bool AutoAssignGroup;

	/** Manually set the group that the particles will be placed in */
	UPROPERTY(EditAnywhere, Category = Phase, meta = (editcondition = "!AutoAssignGroup"))
	int32 Group;

	/** Control whether particles interact with other particles in the same group */
	UPROPERTY(EditAnywhere, Category = Phase)
	bool SelfCollide;

	/** If true then particles will not collide or interact with any particles they overlap in the rest pose */
	UPROPERTY(EditAnywhere, Category = Phase)
	bool IgnoreRestCollisions;

	/** Control whether the particles will generate fluid density constraints when interacting with other fluid particles, note that fluids must also be enabled on the container */
	UPROPERTY(EditAnywhere, Category = Phase)
	bool Fluid;

};