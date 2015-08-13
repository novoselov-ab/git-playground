#include "TurbulenceEditorPCH.h"

/*-----------------------------------------------------------------------------
UVortexAssetFactory.
-----------------------------------------------------------------------------*/
UVortexAssetFactory::UVortexAssetFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

	SupportedClass = UVortexAsset::StaticClass();

	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UVortexAssetFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	UVortexAsset* VortexAsset = ConstructObject<UVortexAsset>(UVortexAsset::StaticClass(), InParent, InName, Flags);

	return VortexAsset;
}
