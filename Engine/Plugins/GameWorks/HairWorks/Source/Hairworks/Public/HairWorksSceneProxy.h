#pragma once

class UHairWorksAsset;

/**
* Hair component scene proxy.
*/
class HAIRWORKS_API FHairWorksSceneProxy : public FPrimitiveSceneProxy
{
public:
	struct FDynamicRenderData
	{
		GFSDK_HairInstanceDescriptor HairInstanceDesc;
		TArray<FMatrix> BoneMatrices;
		//FName NormalCenterBoneName;
		int32 NormalCenterBoneIndex; // Signed int, because -1 means bone not found/set
		TArray<UTexture2D*> Textures;
	};

	FHairWorksSceneProxy(const UPrimitiveComponent* InComponent, GFSDK_HairAssetID InAssetId);
	~FHairWorksSceneProxy();

	uint32 GetMemoryFootprint(void) const override;
	void CreateRenderThreadResources() override;
	FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) override;

	void UpdateDynamicData_RenderThread(TSharedPtr<FDynamicRenderData> DynamicData);

	void DrawBasePass(const FSceneView& View);
	void DrawShadow(const FViewMatrices& ViewMatrices, float DepthBias, float Scale);
	void DrawTranslucency(const FSceneView& View, const FVector& LightDir, const FLinearColor& LightColor, FTextureRHIRef LightAttenuation, const FVector4 IndirectLight[3]);

	// Draw shadows with matrices being set beforehand
	void DrawShadows(FRHICommandList &RHICmdList);
	void DrawVelocity(const FSceneView& View, const FViewMatrices& PrevViewMatrices);


	void DrawToGBuffers(const FSceneView& View);

	bool isHairInstanceNull() { return HairInstanceId == GFSDK_HairInstanceID_NULL; }

	GFSDK_HairInstanceID getHairInstanceID() { return HairInstanceId; }
protected:

	GFSDK_HairAssetID AssetId;

	// The hair
	GFSDK_HairInstanceID HairInstanceId;

	//** Control textures */
	TArray<FTexture2DRHIRef> HairTextures;

	// Bone mapping indices
	TArray<uint16> BoneMapping;
};

