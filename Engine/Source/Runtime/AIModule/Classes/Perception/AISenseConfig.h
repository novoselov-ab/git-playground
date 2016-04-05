// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AIPerceptionTypes.h"
#if !UE_BUILD_SHIPPING
#	include "Debug/GameplayDebuggerBaseObject.h"
#endif
#include "AISenseConfig.generated.h"

class UAISenseImplementation;
class UCanvas;
class UAIPerceptionComponent;

UCLASS(ClassGroup = AI, abstract, EditInlineNew, config=Game)
class AIMODULE_API UAISenseConfig : public UObject
{
	GENERATED_BODY()

protected:
	/** specifies age limit after stimuli generated by this sense become forgotten. 0 means "never" */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sense", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float MaxAge;
	
	/** determines whether given sense starts in an enabled state */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sense")
	uint32 bStartsEnabled : 1;
	
public:
	UAISenseConfig(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual TSubclassOf<UAISense> GetSenseImplementation() const;

	FAISenseID GetSenseID() const;
	
	float GetMaxAge() const { return MaxAge == 0.f ? FAIStimulus::NeverHappenedAge : MaxAge; }
	bool IsEnabled() const { return bStartsEnabled; }

#if !UE_BUILD_SHIPPING
	//----------------------------------------------------------------------//
	// DEBUG	
	//----------------------------------------------------------------------//
	virtual void GetDebugData(TArray<FString>& OnScreenStrings, TArray<FGameplayDebuggerShapeElement>& DebugShapes, const UAIPerceptionComponent& PerceptionComponent) const {}
#endif // !UE_BUILD_SHIPPING
};