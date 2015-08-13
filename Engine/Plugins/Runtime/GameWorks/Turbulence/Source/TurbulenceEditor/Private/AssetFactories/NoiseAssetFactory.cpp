#include "NoiseAssetFactory.h"

/*-----------------------------------------------------------------------------
UNoiseAssetFactory.
-----------------------------------------------------------------------------*/
UNoiseAssetFactory::UNoiseAssetFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

	SupportedClass = UNoiseAsset::StaticClass();

	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UNoiseAssetFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	UNoiseAsset* NoiseAsset = ConstructObject<UNoiseAsset>(UNoiseAsset::StaticClass(), InParent, InName, Flags);

	return NoiseAsset;
}
