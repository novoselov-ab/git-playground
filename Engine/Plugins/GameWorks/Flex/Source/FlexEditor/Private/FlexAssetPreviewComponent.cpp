#include "FlexEditorPCH.h"

UFlexAssetPreviewComponent::UFlexAssetPreviewComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, FlexAsset(NULL)
{}

FPrimitiveSceneProxy* UFlexAssetPreviewComponent::CreateSceneProxy()
{
	return new FFlexAssetPreviewSceneProxy(this);
}

FBoxSphereBounds UFlexAssetPreviewComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	const FVector BoxExtent(HALF_WORLD_MAX);
	return FBoxSphereBounds(FVector::ZeroVector, BoxExtent, BoxExtent.Size());
}
