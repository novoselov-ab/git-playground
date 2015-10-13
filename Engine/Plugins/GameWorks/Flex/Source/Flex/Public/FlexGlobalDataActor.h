#pragma once

#include "FlexGlobalDataActor.generated.h"

/*
	This class will be automatically added to the scene if it doesn't exist, and holds the FlexContainerInstances that are present in the scene.

*/
UCLASS()
class AFlexGlobalDataActor : public AActor
{
	GENERATED_BODY()
public:
	AFlexGlobalDataActor(const FObjectInitializer &ObjectInitializer);
	virtual ~AFlexGlobalDataActor();

	// Returns the unique FFlexContainerInstance that uses this UFlexContainer. If it doesn't exist, it will be created and added to the map first.
	TSharedPtr<FFlexContainerInstance> GetContainerInstance(UFlexContainer *ContainerTypeKey);

	// Helper function that returns the current AFlexGlobalDataActor, or creates one if needed.
	static AFlexGlobalDataActor* GetGlobalDataActorFromWorld(UWorld *World);

	// TODO: Force applications

	// Hook the event at the end of FPhysScene StartFrame() to drive TickFlexScenes
	virtual void PostInitializeComponents() override;

private:

	void TickFlexScenes(float DeltaSeconds);

	TMap<UFlexContainer*, TSharedPtr<FFlexContainerInstance>> FlexContainerMap;

	uint8 PostFrameCallbackHandle;
};