#include "FlexPCH.h"

UFlexAsset::UFlexAsset(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Asset = NULL;
}

void UFlexAsset::PostInitProperties()
{
#if WITH_FLEX
	//JDM: Fix this crap - newing a struct? WTF.
	// allocate an extensions object to represent particles and constraints for this asset
	Asset = new FlexExtAsset();
#endif

	Super::PostInitProperties();
}

void UFlexAsset::BeginDestroy()
{
#if WITH_FLEX
	delete Asset;
#endif

	Super::BeginDestroy();
}

#if WITH_EDITOR
//JDM: TODO - fix this
void UFlexAsset::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	/*
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	if ((PropertyName == FName(TEXT("Mesh"))))
	{
		ReImport();
	}
	*/
}
#endif // WITH_EDITOR









