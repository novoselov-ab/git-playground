#include "TurbulenceEditorPCH.h"

/*-----------------------------------------------------------------------------
UVelocitySourceAssetFactory.
-----------------------------------------------------------------------------*/
UVelocitySourceAssetFactory::UVelocitySourceAssetFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

	SupportedClass = UVelocitySourceAsset::StaticClass();

	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UVelocitySourceAssetFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	UVelocitySourceAsset* VelocitySourceAsset = ConstructObject<UVelocitySourceAsset>(UVelocitySourceAsset::StaticClass(), InParent, InName, Flags);

	return VelocitySourceAsset;
}
