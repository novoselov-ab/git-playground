#pragma once

class FHairWorksLogger;


class HAIRWORKS_API FHairWorksManager
{
public:
	FHairWorksManager();
	~FHairWorksManager();


	void ReleaseHair_GameThread(GFSDK_HairAssetID AssetId);

	bool GetHairInfo(TMap<FName, int32>& OutBoneToIdxMap, UHairWorksAsset* Hair);

	bool IsHair_GameThread(const void* AssetData, unsigned DataSize);

	void RenderTranslucency(const FViewInfo &View, FRHICommandList& RHICmdList);
	void RenderBaseView(FViewInfo &View);

	void UpdateHairFlagsAndClearRenderTargets(TArray<FViewInfo> &Views, FRHICommandList& RHICmdList);

	void RenderToGBuffers(const FViewInfo& View, FRHICommandList& RHICmdList);
	void RenderBasePassDynamic(const FViewInfo& View, FRHICommandList& RHICmdList);

	void SetHairLightSettings(FVector InDirection, FLinearColor InColor, bool InShadow);

	void RenderProjectedShadows(FRHICommandList& RHICmdList, const FProjectedShadowInfo& ShadowInfo, const TArray<const FPrimitiveSceneInfo*, SceneRenderingAllocator> SubjectPrimitives, const FViewInfo *View);
	void RenderDepthDynamic(const FViewInfo* View, TArray<const FPrimitiveSceneInfo*, SceneRenderingAllocator> SubjectPrimitives, FViewMatrices ViewMatrices, float ShaderDepthBias, float InvMaxSubjectDepth);

	void ClearHairLightAttenuation(FRHICommandList &RHICmdList);

	void RenderShadowProjection(const FProjectedShadowInfo& shadowInfo, const FViewInfo& View, FRHICommandList& RHICmdList);

	void PostShadowRender(const FProjectedShadowInfo& shadowInfo, const FViewInfo& View, int32 ViewIndex, FRHICommandListImmediate& RHICmdList);

	void AfterRenderProjection(const FProjectedShadowInfo& shadowInfo, const FViewInfo& View, int32 ViewIndex, FRHICommandListImmediate& RHICmdList);

	void RenderVelocitiesInner(const FViewInfo &View);

	void SortVisibleDynamicPrimitives(FViewInfo &View);

	void StartMsaa();
	void FinishMsaa();
	void DrawPostColor(bool bVelocity = false);
	void DrawPostDepth();

	void StepSimulation();

#if WITH_EDITOR
	// Used for injection of console variables.
	void UpdateHairInstanceDescriptor(GFSDK_HairInstanceID InstanceId, GFSDK_HairInstanceDescriptor& HairDesc);
#endif
	void UpdateViewPreShadow(const FProjectedShadowInfo &ShadowInfo, const FViewInfo &View, const TArray<const FPrimitiveSceneInfo*, SceneRenderingAllocator> &ReceiverPrimitives);

	GFSDK_HairSDK* GetHairworksSdk() { return HairWorksSdk; }

	GFSDK_HairConversionSettings HairWorksConversionSettings;

	// Add light settings
	FVector				LightDirection;
	FLinearColor		LightColor;
	bool				bLightShadowed;

	TAutoConsoleVariable<int> CVarHairMsaaLevel;
	TAutoConsoleVariable<int> CVarHairTemporalAa;
	TAutoConsoleVariable<int> CVarHairMsaa;
	TAutoConsoleVariable<int> CVarHairShadows;
	TAutoConsoleVariable<float> CVarHairOutputVelocity;
	TAutoConsoleVariable<float> CVarHairShadowBiasScale;
	TAutoConsoleVariable<float> CVarHairShadowTransitionScale;
	TAutoConsoleVariable<float> CVarHairShadowWidthScale;
	TAutoConsoleVariable<float> CVarHairShadowTexelsScale;

	TAutoConsoleVariable<int> CVarHairVisualizationHair;
	TAutoConsoleVariable<int> CVarHairVisualizationGuideCurves;
	TAutoConsoleVariable<int> CVarHairVisualizationSkinnedGuideCurves;
	TAutoConsoleVariable<int> CVarHairVisualizationControlPoints;
	TAutoConsoleVariable<int> CVarHairVisualizationGrowthMesh;
	TAutoConsoleVariable<int> CVarHairVisualizationBones;
	TAutoConsoleVariable<int> CVarHairVisualizationBoundingBox;
	TAutoConsoleVariable<int> CVarHairVisualizationCollisionCapsules;
	TAutoConsoleVariable<int> CVarHairVisualizationHairInteraction;
	TAutoConsoleVariable<int> CVarHairVisualizationPinConstraints;
	TAutoConsoleVariable<int> CVarHairVisualizationShadingNormal;
	TAutoConsoleVariable<int> CVarHairVisualizationShadingNormalCenter;

	TRefCountPtr<IPooledRenderTarget> HairMask;
	TRefCountPtr<IPooledRenderTarget> HairLightAttenuation;
	TRefCountPtr<IPooledRenderTarget> HairDepthZ;

private:
	GFSDK_HairSDK* HairWorksSdk;
	TUniquePtr<FHairWorksLogger> HWLogger;

	FDelegateHandle		RHIInitHandle;
	FDelegateHandle		OnExitHandle;

	void FreeResources();
	void PostRHIInitLoad();
	void AllocHairDepthZ(FPooledRenderTargetDesc Desc);
	void AllocHairLightAttenuation(FPooledRenderTargetDesc Desc);
	void AllocHairMask(FPooledRenderTargetDesc Desc);
	void DeallocRenderTargets();
	void LoadSDKDll();
};
