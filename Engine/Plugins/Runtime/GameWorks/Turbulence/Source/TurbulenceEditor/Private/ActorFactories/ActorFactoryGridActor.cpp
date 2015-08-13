#include "TurbulenceEditorPCH.h"

/*-----------------------------------------------------------------------------
UActorFactoryGridActor
-----------------------------------------------------------------------------*/
UActorFactoryGridActor::UActorFactoryGridActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DisplayName = LOCTEXT("GridActorDisplayName", "Grid Actor");
	NewActorClass = AGridActor::StaticClass();
}

bool UActorFactoryGridActor::CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg)
{
	if (!AssetData.IsValid() || !AssetData.GetClass()->IsChildOf(UGridAsset::StaticClass()))
	{
		OutErrorMsg = NSLOCTEXT("CanCreateActor", "NoGridAsset", "No Grid Asset was specified.");
		return false;
	}

	return true;
}

void UActorFactoryGridActor::PostSpawnActor(UObject* Asset, AActor* NewActor)
{
	UGridAsset* GridAsset = CastChecked<UGridAsset>(Asset);
	AGridActor* GridActor = CastChecked<AGridActor>(NewActor);

	if (GridActor && GridActor->FieldSamplerComponent)
	{
		UGridComponent* GridComponent = CastChecked<UGridComponent>(GridActor->FieldSamplerComponent);
		if (GridComponent)
		{
			GridComponent->GridAsset = GridAsset;
			GridActor->PostEditChange();
		}
	}
}