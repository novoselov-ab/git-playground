#include "FlexPCH.h"


AFlexActor::AFlexActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UFlexComponent>(TEXT("StaticMeshComponent0")))
{
	UFlexComponent* FlexMeshComponent = CastChecked<UFlexComponent>(StaticMeshComponent);
	RootComponent = FlexMeshComponent;
	FlexMeshComponent->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	FlexMeshComponent->Mobility = EComponentMobility::Movable;
	FlexMeshComponent->SetSimulatePhysics(false);

	PrimaryActorTick.bCanEverTick = false;
}