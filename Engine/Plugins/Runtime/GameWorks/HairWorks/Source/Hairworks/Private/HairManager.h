#pragma once
#include "RendererHooks.h"

class FHairWorksLogger;

class FHairManager
{
public:
	FHairManager();
	~FHairManager();


	void ReleaseHair_GameThread(GFSDK_HairAssetID AssetId);
	bool GetHairInfo_GameThread(GFSDK_HairInstanceDescriptor& HairDescriptor, TMap<FName, int32>& BoneToIdxMap, const UHair& Hair);
	bool IsHair_GameThread(const void* AssetData, unsigned DataSize);

	void RenderTranslucency(const FViewInfo &View);
	void RenderBaseView(const FViewInfo &View);
	void RenderBasePassDynamic(const FViewInfo& View, FRHICommandList& RHICmdList);
	void SetHairLightSettings(FVector InDirection, FLinearColor InColor, bool InShadow);
	void RenderDepthDynamic(const FViewInfo* View, PrimitiveArrayType SubjectPrimitives, FViewMatrices ViewMatrices, float ShaderDepthBias, float InvMaxSubjectDepth);

	void RenderShadowProjection(const FProjectedShadowInfo& shadowInfo, const FViewInfo& View, FRHICommandList& RHICmdList);

	void RenderVelocitiesInner(const FViewInfo &View);

	void SortVisibleDynamicPrimitives(FViewInfo &View);

	void StartMsaa();
	void FinishMsaa();
	void DrawPostColor(bool bVelocity = false);
	void DrawPostDepth();

	void StepSimulation();

	void UpdateHairInstanceDescriptor(GFSDK_HairInstanceID InstanceId, GFSDK_HairInstanceDescriptor& HairDesc);
	void UpdateViewPreShadow(const FProjectedShadowInfo &ShadowInfo, const FViewInfo &View, PrimitiveArrayType &ReceiverPrimitives);

	GFSDK_HairSDK* GetHairworksSdk() { return HairWorksSdk; }

	// Add light settings
	FVector				LightDirection;
	FLinearColor		LightColor;
	bool				bLightShadowed;

	TAutoConsoleVariable<int> CVarHairMsaaLevel;
	TAutoConsoleVariable<int> CVarHairTemporalAa;
	TAutoConsoleVariable<float> CVarHairOutputVelocity;
	TAutoConsoleVariable<float> CVarHairShadowBiasScale;
	TAutoConsoleVariable<float> CVarHairShadowTransitionScale;
	TAutoConsoleVariable<float> CVarHairShadowWidthScale;
	TAutoConsoleVariable<float> CVarHairShadowTexelsScale;

	TAutoConsoleVariable<int> CVarHairVisualizeGuideHairs;
	TAutoConsoleVariable<int> CVarHairVisualizeSkinnedGuideHairs;
	TAutoConsoleVariable<int> CVarHairVisualizeHairInteractions;
	TAutoConsoleVariable<int> CVarHairVisualizeControlVertices;
	TAutoConsoleVariable<int> CVarHairVisualizeFrames;
	TAutoConsoleVariable<int> CVarHairVisualizeLocalPos;
	TAutoConsoleVariable<int> CVarHairVisualizeShadingNormals;
	TAutoConsoleVariable<int> CVarHairVisualizeGrowthMesh;
	TAutoConsoleVariable<int> CVarHairVisualizeBones;
	TAutoConsoleVariable<int> CVarHairVisualizeCapsules;
	TAutoConsoleVariable<int> CVarHairVisualizeBoundingBox;
	TAutoConsoleVariable<int> CVarHairVisualizePinConstraints;
	TAutoConsoleVariable<int> CVarHairVisualizeShadingNormalBone;

private:
	GFSDK_HairSDK* HairWorksSdk;
	TUniquePtr<FHairWorksLogger> HWLogger;


	FDelegateHandle		RHIInitHandle;
	FDelegateHandle		OnExitHandle;

	void FreeResources();
	void PostRHIInitLoad();
	void LoadSDKDll();
};