#include "TurbulenceEditorPCH.h"


/*-----------------------------------------------------------------------------
UActorFactoryVortexActor
-----------------------------------------------------------------------------*/
UActorFactoryVortexActor::UActorFactoryVortexActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DisplayName = LOCTEXT("VortexActorDisplayName", "Vortex Actor");
	NewActorClass = AVortexActor::StaticClass();
}

bool UActorFactoryVortexActor::CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg)
{
	if (!AssetData.IsValid() || !AssetData.GetClass()->IsChildOf(UVortexAsset::StaticClass()))
	{
		OutErrorMsg = NSLOCTEXT("CanCreateActor", "NoVortexAsset", "No Vortex Asset was specified.");
		return false;
	}

	return true;
}

void UActorFactoryVortexActor::PostSpawnActor(UObject* Asset, AActor* NewActor)
{
	UVortexAsset* VortexAsset = CastChecked<UVortexAsset>(Asset);
	AVortexActor* VortexActor = CastChecked<AVortexActor>(NewActor);

	if (VortexActor && VortexActor->FieldSamplerComponent)
	{
		UVortexComponent* VortexComponent = CastChecked<UVortexComponent>(VortexActor->FieldSamplerComponent);
		if (VortexComponent)
		{
			VortexComponent->VortexAsset = VortexAsset;
			VortexActor->PostEditChange();
		}
	}
}

