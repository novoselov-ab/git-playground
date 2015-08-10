#include "HairworksPCH.h"
#include "RHIStaticStates.h"
#include "D3D11RHIPrivate.h"
#include "RendererPrivate.h"
#include "SceneRendering.h"
#include "PrimitiveSceneInfo.h"
#include "PostProcess/SceneRenderTargets.h"
#include "EngineModule.h"
#include "RendererHooks.h"

using namespace std::placeholders;

FHairManager::FHairManager():
	CVarHairMsaaLevel(TEXT("r.Hair.MsaaLevel"), 4, TEXT(""), ECVF_RenderThreadSafe),
	CVarHairTemporalAa(TEXT("r.Hair.TemporalAa"), 1, TEXT(""), ECVF_RenderThreadSafe),
	CVarHairOutputVelocity(TEXT("r.Hair.OutputVelocity"), 1, TEXT(""), ECVF_RenderThreadSafe),
	CVarHairShadowBiasScale(TEXT("r.Hair.Shadow.BiasScale"), 0.1, TEXT(""), ECVF_RenderThreadSafe),
	CVarHairShadowTransitionScale(TEXT("r.Hair.Shadow.TransitionScale"), 0.1, TEXT(""), ECVF_RenderThreadSafe),
	CVarHairShadowWidthScale(TEXT("r.Hair.Shadow.WidthScale"), 1, TEXT(""), ECVF_RenderThreadSafe),
	CVarHairShadowTexelsScale(TEXT("r.Hair.Shadow.TexelsScale"), 5, TEXT(""), ECVF_RenderThreadSafe),
	CVarHairVisualizeGuideHairs(TEXT("r.Hair.VisualizeGuideHairs"), 0, TEXT(""), ECVF_RenderThreadSafe),
	CVarHairVisualizeSkinnedGuideHairs(TEXT("r.Hair.VisualizeSkinnedGuideHairs"), 0, TEXT(""), ECVF_RenderThreadSafe),
	CVarHairVisualizeHairInteractions(TEXT("r.Hair.VisualizeHairInteractions"), 0, TEXT(""), ECVF_RenderThreadSafe),
	CVarHairVisualizeControlVertices(TEXT("r.Hair.VisualizeControlVertices"), 0, TEXT(""), ECVF_RenderThreadSafe),
	CVarHairVisualizeFrames(TEXT("r.Hair.VisualizeFrames"), 0, TEXT(""), ECVF_RenderThreadSafe),
	CVarHairVisualizeLocalPos(TEXT("r.Hair.VisualizeLocalPos"), 0, TEXT(""), ECVF_RenderThreadSafe),
	CVarHairVisualizeShadingNormals(TEXT("r.Hair.VisualizeShadingNormals"), 0, TEXT(""), ECVF_RenderThreadSafe),
	CVarHairVisualizeGrowthMesh(TEXT("r.Hair.VisualizeGrowthMesh"), 0, TEXT(""), ECVF_RenderThreadSafe),
	CVarHairVisualizeBones(TEXT("r.Hair.VisualizeBones"), 0, TEXT(""), ECVF_RenderThreadSafe),
	CVarHairVisualizeCapsules(TEXT("r.Hair.VisualizeCapsules"), 0, TEXT(""), ECVF_RenderThreadSafe),
	CVarHairVisualizeBoundingBox(TEXT("r.Hair.VisualizeBoundingBox"), 0, TEXT(""), ECVF_RenderThreadSafe),
	CVarHairVisualizePinConstraints(TEXT("r.Hair.VisualizePinConstraints"), 0, TEXT(""), ECVF_RenderThreadSafe),
	CVarHairVisualizeShadingNormalBone(TEXT("r.Hair.VisualizeShadingNormalBone"), 0, TEXT(""), ECVF_RenderThreadSafe),
	HWLogger(MakeUnique<FHairWorksLogger>())
{
	FRendererHooks::get().TranslucentViewRenderCallbacks.Add(std::bind(&FHairManager::RenderTranslucency, this, _1), 0);
	FRendererHooks::get().RenderVelocitiesInnerCallbacks.Add(std::bind(&FHairManager::RenderVelocitiesInner, this, _1), 0);
	FRendererHooks::get().RenderBasePassViewCallbacks.Add(std::bind(&FHairManager::RenderBaseView, this, _1), 0);
	FRendererHooks::get().RenderBasePassDynamicCallbacks.Add(std::bind(&FHairManager::RenderBasePassDynamic, this, _1, _2), 0);
	FRendererHooks::get().RenderProjectedShadowDepthDynamicCallbacks.Add(std::bind(&FHairManager::RenderDepthDynamic, this, _1, _2, _3, _4, _5), 0);
	FRendererHooks::get().RenderProjectedShadowPreShadowCallbacks.Add(std::bind(&FHairManager::UpdateViewPreShadow, this, _1, _2, _3), 0);
	FRendererHooks::get().RenderProjectedShadowRenderProjectionCallbacks.Add(std::bind(&FHairManager::RenderShadowProjection, this, _1, _2, _3), 0);
	FRendererHooks::get().SetHairLightCallbacks.Add(std::bind(&FHairManager::SetHairLightSettings, this, _1, _2, _3), 0);
	FRendererHooks::get().PostVisibilityFrameSetupCallbacks.Add(std::bind(&FHairManager::SortVisibleDynamicPrimitives, this, _1), 0);

	RHIInitHandle = FCoreDelegates::OnRHIInit.AddRaw(this, &FHairManager::PostRHIInitLoad);
	OnExitHandle = FCoreDelegates::OnExit.AddRaw(this, &FHairManager::FreeResources);
}

FHairManager::~FHairManager()
{

}

void FHairManager::PostRHIInitLoad()
{
	FCoreDelegates::OnRHIInit.Remove(RHIInitHandle);

	LoadSDKDll();

	if (HairWorksSdk == nullptr)
	{
		UE_LOG(LogHairWorks, Error, TEXT("Failed to load the HairWorks DLL. Without this no HairWorks features will work."));
	}
}

void FHairManager::StepSimulation()
{
	if (!HairWorksSdk)
		return;

	static uint32 LastFrameNumber = -1;
	if (LastFrameNumber != GFrameNumberRenderThread)
	{
		LastFrameNumber = GFrameNumberRenderThread;

		HairWorksSdk->StepSimulation();
	}
}

void FHairManager::StartMsaa()
{
	if (!HairWorksSdk)
		return;

	FRHICommandListExecutor::GetImmediateCommandList().SetDepthStencilState(TStaticDepthStencilState<false>::GetRHI());	// Render targets may be changed in this function. We need to call this before MSAA.

	HairWorksSdk->StartMSAARendering(CVarHairMsaaLevel.GetValueOnRenderThread(), false);
}

void FHairManager::FinishMsaa()
{
	if (!HairWorksSdk)
		return;

	HairWorksSdk->FinishMSAARendering();
}

void FHairManager::DrawPostColor(bool bVelocity)
{
	if(!HairWorksSdk)
		return;

	HairWorksSdk->DrawMSAAColor(bVelocity);
}

void FHairManager::DrawPostDepth()
{
	if(!HairWorksSdk)
		return;

	HairWorksSdk->DrawMSAAPostDepth();
}

bool FHairManager::IsHair_GameThread(const void* AssetData, unsigned DataSize)
{
	if (!HairWorksSdk)
		return false;

	bool IsHairAsset = false;

	ENQUEUE_UNIQUE_RENDER_COMMAND_THREEPARAMETER(
		IsHairAsset,
		const void*, AssetData, AssetData,
		unsigned, DataSize, DataSize,
		bool&, IsHairAsset, IsHairAsset,
		{
			// Try to create Hair asset
			GFSDK_HairAssetID AssetId = GFSDK_HairAssetID_NULL;

			auto HairWorksSdk = GHairManager->GetHairworksSdk();

			HairWorksSdk->LoadHairAssetFromMemory(AssetData, DataSize, &AssetId);

			// Release created asset
			if (AssetId == GFSDK_HairAssetID_NULL)
				return;

			HairWorksSdk->FreeHairAsset(AssetId);
			IsHairAsset = true;
		}
	);

	FRenderCommandFence RenderCmdFenc;
	RenderCmdFenc.BeginFence();
	RenderCmdFenc.Wait();

	return IsHairAsset;
}

void FHairManager::ReleaseHair_GameThread(GFSDK_HairAssetID AssetId)
{
	if (!HairWorksSdk || AssetId == GFSDK_HairAssetID_NULL)
		return;

	UE_LOG(LogHairWorks, Log, TEXT("Freeing hair asset"));

	HairWorksSdk->FreeHairAsset(AssetId);
}

bool FHairManager::GetHairInfo_GameThread(GFSDK_HairInstanceDescriptor& HairDescriptor, TMap<FName, int32>& BoneToIdxMap, const UHair& Hair)
{
	if (!HairWorksSdk)
		return false;

	UE_LOG(LogHairWorks, Log, TEXT("HM:GetHairInfo_GT"));

	bool bOk = false;

	ENQUEUE_UNIQUE_RENDER_COMMAND_FOURPARAMETER(
		GetHairInfo,
		GFSDK_HairInstanceDescriptor&, HairDescriptor, HairDescriptor,
		decltype(BoneToIdxMap)&, BoneToIdxMap, BoneToIdxMap,	// Use decltype to avoid compile error.
		const UHair&, Hair, Hair,
		bool&, bOk, bOk,
		{
			auto HairWorksSdk = GHairManager->GetHairworksSdk();

			// Try to create HairWorks asset
			auto AssetId = Hair.AssetId;
			if (AssetId == GFSDK_HairAssetID_NULL)
			{
				HairWorksSdk->LoadHairAssetFromMemory(Hair.AssetData.GetData(), Hair.AssetData.Num() * Hair.AssetData.GetTypeSize(), &AssetId);
			}

			if (AssetId == GFSDK_HairAssetID_NULL)
				return;

			// Copy descriptor
			HairWorksSdk->CopyInstanceDescriptorFromAsset(AssetId, HairDescriptor);

			// Copy bones
			gfsdk_U32 BoneNum = 0;
			HairWorksSdk->GetNumBones(AssetId, &BoneNum);

			for (gfsdk_U32 BoneIdx = 0; BoneIdx < BoneNum; ++BoneIdx)
			{
				gfsdk_char BoneName[GFSDK_HAIR_MAX_STRING];
				HairWorksSdk->GetBoneName(AssetId, BoneIdx, BoneName);

				BoneToIdxMap.Add(BoneName, BoneIdx);
			}

			// Release asset
			if (AssetId != Hair.AssetId)
				HairWorksSdk->FreeHairAsset(AssetId);

			bOk = true;
		}
	);

	// Wait
	FRenderCommandFence RenderCmdFenc;
	RenderCmdFenc.BeginFence();
	RenderCmdFenc.Wait();

	return bOk;
}

void FHairManager::LoadSDKDll()
{
	// Check platform
	if (GMaxRHIShaderPlatform != EShaderPlatform::SP_PCD3D_SM5)
		return;

	// Initialize SDK
	FString LibPath = FPaths::EngineDir() / TEXT("Binaries/ThirdParty/HairWorks/GFSDK_HairWorks.win");

#if PLATFORM_64BITS
	LibPath += TEXT("64");
#else
	LibPath += TEXT("32");
#endif

	LibPath += TEXT(".dll");

	HairWorksSdk = GFSDK_LoadHairSDK(TCHAR_TO_ANSI(*LibPath), GFSDK_HAIRWORKS_VERSION, nullptr, HWLogger.Get());
	if (!HairWorksSdk)
		return;

	auto& D3d11Rhi = static_cast<FD3D11DynamicRHI&>(*GDynamicRHI);
	HairWorksSdk->InitRenderResources(D3d11Rhi.GetDevice(), D3d11Rhi.GetDeviceContext());
}

void FHairManager::FreeResources()
{
	FCoreDelegates::OnExit.Remove(OnExitHandle);

	if (HairWorksSdk)
	{
		UE_LOG(LogHairWorks, Log, TEXT("Freeing HairWorks resources"));

		HairWorksSdk->FreeRenderResources();
		HairWorksSdk->Release();
	}
}

void FHairManager::RenderBaseView(const FViewInfo &View)
{
	for (auto MeshIdx = 0; MeshIdx < View.VisibleDynamicPrimitives.Num(); ++MeshIdx)
	{
		auto PrimitiveInfo = View.VisibleDynamicPrimitives[MeshIdx];
		auto& ViewRelevance = View.PrimitiveViewRelevanceMap[PrimitiveInfo->GetIndex()];
		if (ViewRelevance.bHair)
		{
			View.GWData.bHasHair = true;
			break;
		}
	}

}

void FHairManager::RenderTranslucency(const FViewInfo &View)
{
	// Draw hairs in translucency pass
	if (View.GWData.bHasHair)
	{
		StartMsaa();

//		UE_LOG(LogHairWorks, Log, TEXT("HM:RenderTranslucency"));


		for (auto MeshIdx = 0; MeshIdx < View.VisibleDynamicPrimitives.Num(); ++MeshIdx)
		{
			auto& PrimitiveInfo = *View.VisibleDynamicPrimitives[MeshIdx];
			auto& ViewRelevance = View.PrimitiveViewRelevanceMap[PrimitiveInfo.GetIndex()];
			if (!ViewRelevance.GWData.bHair)
				continue;

			// Draw hair
			auto& HairSceneProxy = static_cast<FHairSceneProxy&>(*PrimitiveInfo.Proxy);
			FVector4 IndirectLight[sizeof(FSHVectorRGB2) / sizeof(FVector4)] = { FVector4(0, 0, 0, 0), FVector4(0, 0, 0, 0), FVector4(0, 0, 0, 0) };
			if (PrimitiveInfo.IndirectLightingCacheAllocation
				&& PrimitiveInfo.IndirectLightingCacheAllocation->IsValid()
				&& View.Family->EngineShowFlags.GlobalIllumination)
			{
				const FIndirectLightingCacheAllocation& LightingAllocation = *PrimitiveInfo.IndirectLightingCacheAllocation;
				IndirectLight[0] = LightingAllocation.SingleSamplePacked[0];
				IndirectLight[1] = LightingAllocation.SingleSamplePacked[1];
				IndirectLight[2] = LightingAllocation.SingleSamplePacked[2];
			}

			auto LightAttenuationTexture = GSceneRenderTargets.GWData.HairLightAttenuation && bLightShadowed ? GSceneRenderTargets.GWData.HairLightAttenuation->GetRenderTargetItem().ShaderResourceTexture : nullptr;

			HairSceneProxy.DrawTranslucency(
				View,
				LightDirection,
				LightColor,
				LightAttenuationTexture,
				IndirectLight
				);
		}

		FinishMsaa();

		DrawPostColor();
	}

}

void FHairManager::RenderBasePassDynamic(const FViewInfo& View, FRHICommandList& RHICmdList)
{
	if (View.GWData.bHasHair)
	{
		StartMsaa();

//		UE_LOG(LogHairWorks, Log, TEXT("HM:RenderBasePassDynamic"));

		for (auto MeshIdx = 0; MeshIdx < View.VisibleDynamicPrimitives.Num(); ++MeshIdx)
		{
			auto* PrimitiveInfo = View.VisibleDynamicPrimitives[MeshIdx];
			auto& ViewRelevance = View.PrimitiveViewRelevanceMap[PrimitiveInfo->GetIndex()];
			if (!ViewRelevance.GWData.bHair)
				continue;

			auto& HairSceneProxy = static_cast<FHairSceneProxy&>(*PrimitiveInfo->Proxy);
			HairSceneProxy.DrawBasePass(View);
		}

		FinishMsaa();

		// Write to hair mask buffer and depth buffer.
		SetRenderTarget(RHICmdList, GSceneRenderTargets.GWData.HairMask->GetRenderTargetItem().TargetableTexture, GSceneRenderTargets.GWData.HairDepthZ->GetRenderTargetItem().TargetableTexture, ESimpleRenderTargetMode::EClearToDefault);	// View port is reset here.
		RHICmdList.SetViewport(View.ViewRect.Min.X, View.ViewRect.Min.Y, 0.0f, View.ViewRect.Max.X, View.ViewRect.Max.Y, 1.0f);

		DrawPostDepth();
		DrawPostColor();

		GSceneRenderTargets.BeginRenderingGBuffer(RHICmdList, ERenderTargetLoadAction::ENoAction, ERenderTargetLoadAction::ENoAction);
	}

}

void FHairManager::SetHairLightSettings(FVector InDirection, FLinearColor InColor, bool InShadow)
{
	LightDirection = InDirection;
	LightColor = InColor;
	bLightShadowed = InShadow;
}

void FHairManager::RenderDepthDynamic(const FViewInfo* View, PrimitiveArrayType SubjectPrimitives, FViewMatrices ViewMatrices, float ShaderDepthBias, float InvMaxSubjectDepth)
{
//	UE_LOG(LogHairWorks, Log, TEXT("HM:RenderDepthDynamic"));

	// Draw hairs.
	for (auto PrimitiveIdx = 0; PrimitiveIdx < SubjectPrimitives.Num(); ++PrimitiveIdx)
	{
		auto* PrimitiveInfo = SubjectPrimitives[PrimitiveIdx];
		auto& ViewRelevance = View->PrimitiveViewRelevanceMap[PrimitiveInfo->GetIndex()];
		if (!ViewRelevance.GWData.bHair)
			continue;

		auto& HairProxy = static_cast<FHairSceneProxy&>(*PrimitiveInfo->Proxy);
		HairProxy.DrawShadow(ViewMatrices, ShaderDepthBias, InvMaxSubjectDepth);
	}

}

void FHairManager::RenderShadowProjection(const FProjectedShadowInfo& shadowInfo, const FViewInfo& View, FRHICommandList& RHICmdList)
{
	if (shadowInfo.GWData.bHairReceiver)
	{
		RHICmdList.SetDepthStencilState(TStaticDepthStencilState<false, CF_LessEqual>::GetRHI());

		// Swap to replace render targets as well as shader resources.
		GSceneRenderTargets.LightAttenuation.Swap(GSceneRenderTargets.HairLightAttenuation);
		GSceneRenderTargets.SceneDepthZ.Swap(GSceneRenderTargets.HairDepthZ);
		GSceneRenderTargets.BeginRenderingLightAttenuation(RHICmdList);

		RHICmdList.SetViewport(View->ViewRect.Min.X, View->ViewRect.Min.Y, 0.0f, View->ViewRect.Max.X, View->ViewRect.Max.Y, 1.0f);
	}

}

void FHairManager::RenderVelocitiesInner(const FViewInfo &View)
{
	StartMsaa();

//	UE_LOG(LogHairWorks, Log, TEXT("HM:RenderVelocitiesInner"));

	for (auto PrimitiveIdx = 0; PrimitiveIdx < View.VisibleDynamicPrimitives.Num(); ++PrimitiveIdx)
	{
		auto& PrimitiveInfo = *View.VisibleDynamicPrimitives[PrimitiveIdx];
		auto& ViewRelevance = View.PrimitiveViewRelevanceMap[PrimitiveInfo.GetIndex()];
		if (!ViewRelevance.GWData.bHair)
			continue;

		// Draw hair
		auto& HairSceneProxy = static_cast<FHairSceneProxy&>(*PrimitiveInfo.Proxy);
		HairSceneProxy.DrawVelocity(View, View.PrevViewMatrices);
	}

	FinishMsaa();

	DrawPostColor(true);

}

void FHairManager::SortVisibleDynamicPrimitives(FViewInfo &View)
{
	View.VisibleDynamicPrimitives.Sort([&](const FPrimitiveSceneInfo& Left, const FPrimitiveSceneInfo& Right)
	{
		const FPrimitiveViewRelevance& LeftRelevance = View.PrimitiveViewRelevanceMap[Left.GetIndex()];
		const FPrimitiveViewRelevance& RightRelevance = View.PrimitiveViewRelevanceMap[Right.GetIndex()];
		if (LeftRelevance.GWData.bHair && !RightRelevance.GWData.bHair)
			return false;
		else
			return true;
	}
	);
}

void FHairManager::UpdateHairInstanceDescriptor(GFSDK_HairInstanceID InstanceId, GFSDK_HairInstanceDescriptor& HairDesc)
{
#define HairVisualizerCVarUpdate(name)	\
	HairDesc.m_visualize##name = CVarHairVisualize##name.GetValueOnRenderThread() != 0

	HairVisualizerCVarUpdate(GuideHairs);
	HairVisualizerCVarUpdate(SkinnedGuideHairs);
	HairVisualizerCVarUpdate(HairInteractions);
	HairVisualizerCVarUpdate(ControlVertices);
	HairVisualizerCVarUpdate(Frames);
	HairVisualizerCVarUpdate(LocalPos);
	HairVisualizerCVarUpdate(ShadingNormals);
	HairVisualizerCVarUpdate(GrowthMesh);
	HairVisualizerCVarUpdate(Bones);
	HairVisualizerCVarUpdate(Capsules);
	HairVisualizerCVarUpdate(BoundingBox);
	HairVisualizerCVarUpdate(PinConstraints);
	HairVisualizerCVarUpdate(ShadingNormalBone);

	HairWorksSdk->UpdateInstanceDescriptor(InstanceId, HairDesc);

}

void FHairManager::UpdateViewPreShadow(const FProjectedShadowInfo &ShadowInfo, const FViewInfo &View, PrimitiveArrayType &ReceiverPrimitives)
{
	if (View->bHasHair)
	{
		for (auto PrimitiveSceneInfo : ReceiverPrimitives)
		{
			auto& ViewRelevance = View->PrimitiveViewRelevanceMap[PrimitiveSceneInfo->GetIndex()];
			if (ViewRelevance.bHair)
			{
				checkSlow(!ShadowInfo.GWData.bHairReceiver);
				ShadowInfo.GWData.bHairReceiver = true;
				break;
			}
		}
	}

}