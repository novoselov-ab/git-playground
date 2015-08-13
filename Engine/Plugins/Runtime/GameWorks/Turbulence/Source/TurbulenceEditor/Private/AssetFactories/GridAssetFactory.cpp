#include "TurbulenceEditorPCH.h"

/*-----------------------------------------------------------------------------
UGridAssetFactory.
-----------------------------------------------------------------------------*/
UGridAssetFactory::UGridAssetFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

	SupportedClass = UGridAsset::StaticClass();

	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UGridAssetFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	UGridAsset* GridAsset = ConstructObject<UGridAsset>(UGridAsset::StaticClass(), InParent, InName, Flags);

	return GridAsset;
}
