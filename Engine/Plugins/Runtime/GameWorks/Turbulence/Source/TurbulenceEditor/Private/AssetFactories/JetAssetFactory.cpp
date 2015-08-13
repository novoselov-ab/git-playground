#include "TurbulenceEditorPCH.h"

/*-----------------------------------------------------------------------------
UJetAssetFactory.
-----------------------------------------------------------------------------*/
UJetAssetFactory::UJetAssetFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

	SupportedClass = UJetAsset::StaticClass();

	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UJetAssetFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	UJetAsset* JetAsset = ConstructObject<UJetAsset>(UJetAsset::StaticClass(), InParent, InName, Flags);

	return JetAsset;
}
