#include "TurbulenceEditorPCH.h"

/*-----------------------------------------------------------------------------
UActorFactoryVelocitySourceActor
-----------------------------------------------------------------------------*/
UActorFactoryVelocitySourceActor::UActorFactoryVelocitySourceActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DisplayName = LOCTEXT("VelocitySourceActorDisplayName", "Velocity Source Actor");
	NewActorClass = AVelocitySourceActor::StaticClass();
}

bool UActorFactoryVelocitySourceActor::CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg)
{
	if (!AssetData.IsValid() || !AssetData.GetClass()->IsChildOf(UVelocitySourceAsset::StaticClass()))
	{
		OutErrorMsg = NSLOCTEXT("CanCreateActor", "NoVelocitySourceAsset", "No Velocity Source Asset was specified.");
		return false;
	}

	return true;
}

void UActorFactoryVelocitySourceActor::PostSpawnActor(UObject* Asset, AActor* NewActor)
{
	UVelocitySourceAsset* VelocitySourceAsset = CastChecked<UVelocitySourceAsset>(Asset);
	AVelocitySourceActor* VelocitySourceActor = CastChecked<AVelocitySourceActor>(NewActor);

	if (VelocitySourceActor && VelocitySourceActor->FieldSamplerComponent)
	{
		UVelocitySourceComponent* VelocitySourceComponent = CastChecked<UVelocitySourceComponent>(VelocitySourceActor->FieldSamplerComponent);
		if (VelocitySourceComponent)
		{
			VelocitySourceComponent->VelocitySourceAsset = VelocitySourceAsset;
			VelocitySourceActor->PostEditChange();
		}
	}
}
