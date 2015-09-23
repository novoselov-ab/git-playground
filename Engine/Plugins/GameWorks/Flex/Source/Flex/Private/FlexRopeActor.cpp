#include "FlexPCH.h"

AFlexRopeActor::AFlexRopeActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	FlexRopeComponent = ObjectInitializer.CreateDefaultSubobject<UFlexRopeComponent>(this, TEXT("FlexRopeComponent0"));
	RootComponent = FlexRopeComponent;
}