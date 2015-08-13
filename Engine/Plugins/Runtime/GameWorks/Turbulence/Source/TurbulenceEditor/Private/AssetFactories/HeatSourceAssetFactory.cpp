#include "TurbulenceEditorPCH.h"

/*-----------------------------------------------------------------------------
UHeatSourceAssetFactory.
-----------------------------------------------------------------------------*/
UHeatSourceAssetFactory::UHeatSourceAssetFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

	SupportedClass = UHeatSourceAsset::StaticClass();

	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UHeatSourceAssetFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	UHeatSourceAsset* HeatSourceAsset = ConstructObject<UHeatSourceAsset>(UHeatSourceAsset::StaticClass(), InParent, InName, Flags);

	return HeatSourceAsset;
}
