#include "TurbulenceEditorPCH.h"

/*-----------------------------------------------------------------------------
UAttractorAssetFactory.
-----------------------------------------------------------------------------*/
UAttractorAssetFactory::UAttractorAssetFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

	SupportedClass = UAttractorAsset::StaticClass();

	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UAttractorAssetFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	UAttractorAsset* AttractorAsset = ConstructObject<UAttractorAsset>(UAttractorAsset::StaticClass(), InParent, InName, Flags);

	return AttractorAsset;
}

