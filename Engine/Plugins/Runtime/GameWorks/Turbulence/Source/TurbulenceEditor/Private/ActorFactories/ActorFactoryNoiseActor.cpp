#include "ActorFactoryNoiseActor.h"

/*-----------------------------------------------------------------------------
UActorFactoryNoiseActor
-----------------------------------------------------------------------------*/
UActorFactoryNoiseActor::UActorFactoryNoiseActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DisplayName = LOCTEXT("NoiseActorDisplayName", "Noise Actor");
	NewActorClass = ANoiseActor::StaticClass();
}

bool UActorFactoryNoiseActor::CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg)
{
	if (!AssetData.IsValid() || !AssetData.GetClass()->IsChildOf(UNoiseAsset::StaticClass()))
	{
		OutErrorMsg = NSLOCTEXT("CanCreateActor", "NoNoiseAsset", "No Noise Asset was specified.");
		return false;
	}

	return true;
}

void UActorFactoryNoiseActor::PostSpawnActor(UObject* Asset, AActor* NewActor)
{
	UNoiseAsset* NoiseAsset = CastChecked<UNoiseAsset>(Asset);
	ANoiseActor* NoiseActor = CastChecked<ANoiseActor>(NewActor);

	if (NoiseActor && NoiseActor->FieldSamplerComponent)
	{
		UNoiseComponent* NoiseComponent = CastChecked<UNoiseComponent>(NoiseActor->FieldSamplerComponent);
		if (NoiseComponent)
		{
			NoiseComponent->NoiseAsset = NoiseAsset;
			NoiseActor->PostEditChange();
		}
	}
}
