#pragma once

class UHair;

/**
* Hair component scene proxy.
*/
class HAIRWORKS_API FHairSceneProxy : public FPrimitiveSceneProxy
{
public:
	FHairSceneProxy(const UPrimitiveComponent* InComponent, GFSDK_HairAssetID InAssetId);
	~FHairSceneProxy();

	uint32 GetMemoryFootprint(void) const override;
	void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, class FMeshElementCollector& Collector) const override;
	void CreateRenderThreadResources() override;
	FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) override;

	void SetupBoneMapping_GameThread(const TArray<FMeshBoneInfo>& Bones);
	bool GetHairBounds_GameThread(FBoxSphereBounds& Bounds)const;
	void UpdateBones_GameThread(USkinnedMeshComponent& ParentSkeleton);

	void DrawBasePass(const FSceneView& View);
	void DrawShadow(const FViewMatrices& ViewMatrices, float DepthBias, float Scale);
	void DrawTranslucency(const FSceneView& View, const FVector& LightDir, const FLinearColor& LightColor, FTextureRHIRef LightAttenuation, const FVector4 IndirectLight[3]);
	void DrawVelocity(const FSceneView& View, const FViewMatrices& PrevViewMatrices);

	// This is called from UHairComponent when something changes in the config, to update the cached copies the proxy has.
	void UpdateHairParams(GFSDK_HairInstanceDescriptor& HairDesc, const TArray<FTexture2DRHIRef>& InHairTextures);


	// This is a cached copy of the hair descriptor that's updated every time HairProperties changes.
	GFSDK_HairInstanceDescriptor	CachedHairDescriptor;

	// Control textures
	TArray<FTexture2DRHIRef> HairTextures;
protected:
	void SetupBoneMapping_RenderThread(const TArray<FMeshBoneInfo>& Bones);
	void UpdateBones_RenderThread(const TArray<FMatrix>& RefMatrices);

	// Called when FHairProperties changes on the component.
	void UpdateShaderCache();


	GFSDK_HairAssetID AssetId;

	// The hair
	GFSDK_HairInstanceID HairInstanceId;


	// Bone mapping indices
	TArray<int> BoneMapping;
};