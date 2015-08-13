#include "TurbulenceEditorPCH.h"

/*-----------------------------------------------------------------------------
UActorFactoryJetActor
-----------------------------------------------------------------------------*/
UActorFactoryJetActor::UActorFactoryJetActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DisplayName = LOCTEXT("JetActorDisplayName", "Jet Actor");
	NewActorClass = AJetActor::StaticClass();
}

bool UActorFactoryJetActor::CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg)
{
	if (!AssetData.IsValid() || !AssetData.GetClass()->IsChildOf(UJetAsset::StaticClass()))
	{
		OutErrorMsg = NSLOCTEXT("CanCreateActor", "NoJetAsset", "No Jet Asset was specified.");
		return false;
	}

	return true;
}

void UActorFactoryJetActor::PostSpawnActor(UObject* Asset, AActor* NewActor)
{
	UJetAsset* JetAsset = CastChecked<UJetAsset>(Asset);
	AJetActor* JetActor = CastChecked<AJetActor>(NewActor);

	if (JetActor && JetActor->FieldSamplerComponent)
	{
		UJetComponent* JetComponent = CastChecked<UJetComponent>(JetActor->FieldSamplerComponent);
		if (JetComponent)
		{
			JetComponent->JetAsset = JetAsset;
			JetActor->PostEditChange();
		}
	}
}