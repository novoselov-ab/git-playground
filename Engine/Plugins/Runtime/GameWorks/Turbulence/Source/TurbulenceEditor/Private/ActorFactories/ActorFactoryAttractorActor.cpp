#include "TurbulenceEditorPCH.h"

/*-----------------------------------------------------------------------------
UActorFactoryAttractorActor
-----------------------------------------------------------------------------*/
UActorFactoryAttractorActor::UActorFactoryAttractorActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DisplayName = LOCTEXT("AttractorActorDisplayName", "Attractor Actor");
	NewActorClass = AAttractorActor::StaticClass();
}

bool UActorFactoryAttractorActor::CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg)
{
	if (!AssetData.IsValid() || !AssetData.GetClass()->IsChildOf(UAttractorAsset::StaticClass()))
	{
		OutErrorMsg = NSLOCTEXT("CanCreateActor", "NoAttractorAsset", "No Attractor Asset was specified.");
		return false;
	}

	return true;
}

void UActorFactoryAttractorActor::PostSpawnActor(UObject* Asset, AActor* NewActor)
{
	UAttractorAsset* AttractorAsset = CastChecked<UAttractorAsset>(Asset);
	AAttractorActor* AttractorActor = CastChecked<AAttractorActor>(NewActor);

	if (AttractorActor && AttractorActor->FieldSamplerComponent)
	{
		UAttractorComponent* AttractorComponent = CastChecked<UAttractorComponent>(AttractorActor->FieldSamplerComponent);
		if (AttractorComponent)
		{
			AttractorComponent->AttractorAsset = AttractorAsset;
			AttractorActor->PostEditChange();
		}
	}
}
