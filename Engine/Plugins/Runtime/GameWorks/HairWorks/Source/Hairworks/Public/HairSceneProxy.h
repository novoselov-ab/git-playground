#pragma once

class UHair;

/**
* Hair compoenent scene proxy.
*/
class HAIRWORKS_API FHairSceneProxy : public FPrimitiveSceneProxy
{
public:
	FHairSceneProxy(const UPrimitiveComponent* InComponent, UHair* Hair);
	~FHairSceneProxy();

	uint32 GetMemoryFootprint(void) const override;
	void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, class FMeshElementCollector& Collector) const override;
	void CreateRenderThreadResources() override;
	FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) override;

	void SetupBoneMapping_GameThread(const TArray<FMeshBoneInfo>& Bones);
	bool GetHairBounds_GameThread(FBoxSphereBounds& Bounds)const;
	void UpdateBones_GameThread(USkinnedMeshComponent& ParentSkeleton);
	void UpdateHairParams_GameThread(const GFSDK_HairInstanceDescriptor& HairDesc, const TArray<FTexture2DRHIRef>& HairTextures);

	void DrawBasePass(const FSceneView& View);
	void DrawShadow(const FViewMatrices& ViewMatrices, float DepthBias, float Scale);
	void DrawTranslucency(const FSceneView& View, const FVector& LightDir, const FLinearColor& LightColor, FTextureRHIRef LightAttenuation, const FVector4 IndirectLight[3]);
	void DrawVelocity(const FSceneView& View, const FViewMatrices& PrevViewMatrices);

protected:
	void SetupBoneMapping_RenderThread(const TArray<FMeshBoneInfo>& Bones);
	void UpdateBones_RenderThread(const TArray<FMatrix>& RefMatrices);
	void UpdateHairParams_RenderThread(const GFSDK_HairInstanceDescriptor& HairDesc, const TArray<FTexture2DRHIRef>& HairTextures);
	void UpdateShaderCache();
	static void StepSimulation();

	//JDM: Shared ptr?
	// The APEX asset data
	UHair* Hair;

	// The hair
	GFSDK_HairInstanceID HairInstanceId = GFSDK_HairInstanceID_NULL;

	// Hair parameters
	GFSDK_HairInstanceDescriptor HairDesc;

	// Control textures
	TArray<FTexture2DRHIRef> HairTextures;

	// Bone mapping indices
	TArray<int> BoneMapping;
};