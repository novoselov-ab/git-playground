#include "TurbulenceEditorPCH.h"

/*-----------------------------------------------------------------------------
UActorFactoryHeatSourceActor
-----------------------------------------------------------------------------*/
UActorFactoryHeatSourceActor::UActorFactoryHeatSourceActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DisplayName = LOCTEXT("HeatSourceActorDisplayName", "Heat Source Actor");
	NewActorClass = AHeatSourceActor::StaticClass();
}

bool UActorFactoryHeatSourceActor::CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg)
{
	if (!AssetData.IsValid() || !AssetData.GetClass()->IsChildOf(UHeatSourceAsset::StaticClass()))
	{
		OutErrorMsg = NSLOCTEXT("CanCreateActor", "NoHeatSourceAsset", "No Heat Source Asset was specified.");
		return false;
	}

	return true;
}

void UActorFactoryHeatSourceActor::PostSpawnActor(UObject* Asset, AActor* NewActor)
{
	UHeatSourceAsset* HeatSourceAsset = CastChecked<UHeatSourceAsset>(Asset);
	AHeatSourceActor* HeatSourceActor = CastChecked<AHeatSourceActor>(NewActor);

	if (HeatSourceActor && HeatSourceActor->FieldSamplerComponent)
	{
		UHeatSourceComponent* HeatSourceComponent = CastChecked<UHeatSourceComponent>(HeatSourceActor->FieldSamplerComponent);
		if (HeatSourceComponent)
		{
			HeatSourceComponent->HeatSourceAsset = HeatSourceAsset;
			HeatSourceActor->PostEditChange();
		}
	}
}
