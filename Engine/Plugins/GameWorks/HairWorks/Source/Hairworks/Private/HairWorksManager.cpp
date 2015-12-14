#include "HairworksPCH.h"
#include "RHIStaticStates.h"
#include "D3D11RHIPrivate.h"
#include "RendererPrivate.h"
#include "SceneRendering.h"
#include "PrimitiveSceneInfo.h"
#include "PostProcess/SceneRenderTargets.h"
#include "EngineModule.h"
#include "RendererHooks.h"
#include "ScreenRendering.h"
#include "SceneUtils.h"

using namespace std::placeholders;

FHairWorksManager::FHairWorksManager() :
	CVarHairShadows(TEXT("r.Hair.Shadows"), 1, TEXT(""), ECVF_RenderThreadSafe),
	CVarHairOutputVelocity(TEXT("r.Hair.OutputVelocity"), 1, TEXT(""), ECVF_RenderThreadSafe),
	CVarHairShadowBiasScale(TEXT("r.Hair.Shadow.BiasScale"), 0.1, TEXT(""), ECVF_RenderThreadSafe),
	CVarHairShadowTransitionScale(TEXT("r.Hair.Shadow.TransitionScale"), 0.1, TEXT(""), ECVF_RenderThreadSafe),
	CVarHairShadowWidthScale(TEXT("r.Hair.Shadow.WidthScale"), 1, TEXT(""), ECVF_RenderThreadSafe),
	CVarHairShadowTexelsScale(TEXT("r.Hair.Shadow.TexelsScale"), 5, TEXT(""), ECVF_RenderThreadSafe),

	CVarHairVisualizationHair(TEXT("r.HairWorks.Visualization.Hair"), 1, TEXT(""), ECVF_RenderThreadSafe),
	CVarHairVisualizationGuideCurves(TEXT("r.HairWorks.Visualization.GuideCurves"), 0, TEXT(""), ECVF_RenderThreadSafe),
	CVarHairVisualizationSkinnedGuideCurves(TEXT("r.HairWorks.Visualization.SkinnedGuideCurves"), 0, TEXT(""), ECVF_RenderThreadSafe),
	CVarHairVisualizationControlPoints(TEXT("r.HairWorks.Visualization.ControlPoints"), 0, TEXT(""), ECVF_RenderThreadSafe),
	CVarHairVisualizationGrowthMesh(TEXT("r.HairWorks.Visualization.GrowthMesh"), 0, TEXT(""), ECVF_RenderThreadSafe),
	CVarHairVisualizationBones(TEXT("r.HairWorks.Visualization.Bones"), 0, TEXT(""), ECVF_RenderThreadSafe),
	CVarHairVisualizationBoundingBox(TEXT("r.HairWorks.Visualization.BoundingBox"), 0, TEXT(""), ECVF_RenderThreadSafe),
	CVarHairVisualizationCollisionCapsules(TEXT("r.HairWorks.Visualization.CollisionCapsules"), 0, TEXT(""), ECVF_RenderThreadSafe),
	CVarHairVisualizationHairInteraction(TEXT("r.HairWorks.Visualization.HairInteraction"), 0, TEXT(""), ECVF_RenderThreadSafe),
	CVarHairVisualizationPinConstraints(TEXT("r.HairWorks.Visualization.PinConstraints"), 0, TEXT(""), ECVF_RenderThreadSafe),
	CVarHairVisualizationShadingNormal(TEXT("r.HairWorks.Visualization.ShadingNormal"), 0, TEXT(""), ECVF_RenderThreadSafe),
	CVarHairVisualizationShadingNormalCenter(TEXT("r.HairWorks.Visualization.ShadingNormalCenter"), 0, TEXT(""), ECVF_RenderThreadSafe),
	HairWorksSdk(nullptr),
	HWLogger(MakeUnique<FHairWorksLogger>())
{
//	FRendererHooks::get().TranslucentViewRenderCallbacks.Add(std::bind(&FHairManager::RenderTranslucency, this, _1, _2), 0);
//	FRendererHooks::get().RenderVelocitiesInnerCallbacks.Add(std::bind(&FHairWorksManager::RenderVelocitiesInner, this, _1), 0);

	FRendererHooks::get().RenderMidPointCallbacks.Add(std::bind(&FHairWorksManager::UpdateHairFlagsAndClearRenderTargets, this, _1, _2), 0);

//	FRendererHooks::get().RenderBasePassViewCallbacks.Add(std::bind(&FHairManager::RenderBaseView, this, _1), 0);
	FRendererHooks::get().RenderBasePassDynamicCallbacks.Add(std::bind(&FHairWorksManager::RenderToGBuffers, this, _1, _2), 0);
	FRendererHooks::get().RenderProjectedShadowDepthDynamicCallbacks.Add(std::bind(&FHairWorksManager::RenderDepthDynamic, this, _1, _2, _3, _4, _5), 0);

	FRendererHooks::get().PostVisibilityFrameSetupCallbacks.Add(std::bind(&FHairWorksManager::SortVisibleDynamicPrimitives, this, _1), 0);

	// These are all for shadows, and don't seem to be required!
// 	FRendererHooks::get().FirstShadowRenderCallbacks.Add(std::bind(&FHairManager::ClearHairLightAttenuation, this, _1), 0);
 	FRendererHooks::get().RenderProjectedShadowPreShadowCallbacks.Add(std::bind(&FHairWorksManager::UpdateViewPreShadow, this, _1, _2, _3), 0);
// 	FRendererHooks::get().RenderProjectedShadowRenderProjectionCallbacks.Add(std::bind(&FHairManager::RenderShadowProjection, this, _1, _2, _3), 0);
// 	FRendererHooks::get().RenderProjectedShadowRenderProjectionEndCallbacks.Add(std::bind(&FHairManager::PostShadowRender, this, _1, _2, _3, _4), 0);

	FRendererHooks::get().RenderProjectedShadowCallbacks.Add(std::bind(&FHairWorksManager::RenderProjectedShadows, this, _1, _2, _3, _4), 0);
 	FRendererHooks::get().AfterRenderProjectionCallbacks.Add(std::bind(&FHairWorksManager::AfterRenderProjection, this, _1, _2, _3, _4), 0);
// 	FRendererHooks::get().AllocCommonDepthTargetsCallbacks.Add(std::bind(&FHairManager::AllocHairDepthZ, this, _1), 0);
// 	FRendererHooks::get().AllocLightAttenuationCallbacks.Add(std::bind(&FHairManager::AllocHairLightAttenuation, this, _1), 0);
// 	FRendererHooks::get().AllocSceneColorCallbacks.Add(std::bind(&FHairManager::AllocHairMask, this, _1), 0);
// 	FRendererHooks::get().DeallocRenderTargetsCallbacks.Add(std::bind(&FHairManager::DeallocRenderTargets, this), 0);

	RHIInitHandle = FCoreDelegates::OnRHIInit.AddRaw(this, &FHairWorksManager::PostRHIInitLoad);
	OnExitHandle = FCoreDelegates::OnExit.AddRaw(this, &FHairWorksManager::FreeResources);

	AddAlternateShaderPath(TEXT("Plugins/GameWorks/HairWorks/Shaders"));
}

FHairWorksManager::~FHairWorksManager()
{
//	GWorld->GetTimerManager().ClearTimer(SimTimerHandle);
}

void FHairWorksManager::PostRHIInitLoad()
{
	FCoreDelegates::OnRHIInit.Remove(RHIInitHandle);

	LoadSDKDll();

	if (HairWorksSdk == nullptr)
	{
		UE_LOG(LogHairWorks, Error, TEXT("Failed to load the HairWorks DLL. Without this no HairWorks features will work."));
	}
}



void FHairWorksManager::AllocHairDepthZ(FPooledRenderTargetDesc Desc)
{
	GetRendererModule().RenderTargetPoolFindFreeElement(Desc, HairDepthZ, TEXT("HairDepthZ"));
}

void FHairWorksManager::AllocHairLightAttenuation(FPooledRenderTargetDesc Desc)
{
	GetRendererModule().RenderTargetPoolFindFreeElement(Desc, HairLightAttenuation, TEXT("HairLightAttenuation"));
}

void FHairWorksManager::AllocHairMask(FPooledRenderTargetDesc Desc)
{
	GetRendererModule().RenderTargetPoolFindFreeElement(Desc, HairMask, TEXT("HairMask"));
}

void FHairWorksManager::DeallocRenderTargets()
{
	HairDepthZ.SafeRelease();
	HairLightAttenuation.SafeRelease();
}

void FHairWorksManager::StepSimulation()
{
	if (!HairWorksSdk)
		return;

	static uint32 LastFrameNumber = -1;
//	if (LastFrameNumber == -1)
	if (LastFrameNumber != GFrameNumberRenderThread)
	{
		LastFrameNumber = GFrameNumberRenderThread;

		HairWorksSdk->StepSimulation();
	}
}

bool FHairWorksManager::IsHair_GameThread(const void* AssetData, unsigned DataSize)
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

void FHairWorksManager::ReleaseHair_GameThread(GFSDK_HairAssetID AssetId)
{
	if (!HairWorksSdk || AssetId == GFSDK_HairAssetID_NULL)
		return;

	UE_LOG(LogHairWorks, Log, TEXT("Freeing hair asset"));

	HairWorksSdk->FreeHairAsset(AssetId);
}

bool FHairWorksManager::GetHairInfo(TMap<FName, int32>& OutBoneToIdxMap, UHairWorksAsset* Hair)
{
	check(HairWorksSdk);

	// Try to create HairWorks asset
	auto AssetId = Hair->AssetId;
	if (AssetId == GFSDK_HairAssetID_NULL)
	{
		// This loads the asset on the HW side and assigns it an asset id.
		HairWorksSdk->LoadHairAssetFromMemory(Hair->AssetData.GetData(), Hair->AssetData.Num() * Hair->AssetData.GetTypeSize(), &AssetId);
	}

	if (AssetId == GFSDK_HairAssetID_NULL)
	{
		UE_LOG(LogHairWorks, Error, TEXT("Tried to load a hair asset, but it returned a NULL asset id"));
		return false;
	}

	GFSDK_HairInstanceDescriptor HairDescriptor;

	// Copy descriptor
	HairWorksSdk->CopyInstanceDescriptorFromAsset(AssetId, HairDescriptor);

	//TODO: FIX ME
// 	// Copy those values into the UE4 version of this struct.
// 	Hair->HairProperties.ImportPropertiesFrom(HairDescriptor);

	// Copy bones

	gfsdk_U32 BoneNum = 0;
	HairWorksSdk->GetNumBones(AssetId, &BoneNum);

	// Make sure the old map is empty.
	OutBoneToIdxMap.Empty(BoneNum);

	for (gfsdk_U32 BoneIdx = 0; BoneIdx < BoneNum; ++BoneIdx)
	{
		gfsdk_char BoneName[GFSDK_HAIR_MAX_STRING];
		HairWorksSdk->GetBoneName(AssetId, BoneIdx, BoneName);

		OutBoneToIdxMap.Add(BoneName, BoneIdx);
	}

	Hair->AssetId = AssetId;

	return true;
}



void FHairWorksManager::LoadSDKDll()
{
	// Check platform
	if (GMaxRHIShaderPlatform != EShaderPlatform::SP_PCD3D_SM5)
		return;

	// Initialize SDK
	FString LibPath;

	//TODO: Load debug version properly

#if PLATFORM_64BITS
	LibPath = FPaths::EngineDir() / TEXT("Plugins/GameWorks/HairWorks/Libraries/Win64/GFSDK_HairWorks.win64.dll");
#else
	LibPath = FPaths::EngineDir() / TEXT("Plugins/GameWorks/HairWorks/Libraries/Win64/GFSDK_HairWorks.win32.dll");
#endif

	HairWorksSdk = GFSDK_LoadHairSDK(TCHAR_TO_ANSI(*LibPath), GFSDK_HAIRWORKS_VERSION, nullptr, HWLogger.Get());
	if (!HairWorksSdk)
	{
		UE_LOG(LogHairWorks, Error, TEXT("Failed to load the hairworks DLL at %s"), *LibPath);
		return;
	}

	auto D3d11Rhi = static_cast<FD3D11DynamicRHI *>(GDynamicRHI);

	if (D3d11Rhi == nullptr)
	{
		UE_LOG(LogHairWorks, Error, TEXT("D3D11RHI is null"));
		return;
	}

	HairWorksSdk->InitRenderResources(D3d11Rhi->GetDevice(), D3d11Rhi->GetDeviceContext());
}

void FHairWorksManager::FreeResources()
{
	FCoreDelegates::OnExit.Remove(OnExitHandle);

	if (HairWorksSdk)
	{
		UE_LOG(LogHairWorks, Log, TEXT("Freeing HairWorks resources"));

		HairWorksSdk->FreeRenderResources();
		HairWorksSdk->Release();
	}
}

void FHairWorksManager::RenderBaseView(FViewInfo &View)
{
	for (auto MeshIdx = 0; MeshIdx < View.VisibleDynamicPrimitives.Num(); ++MeshIdx)
	{
		const FPrimitiveSceneInfo* PrimitiveInfo = View.VisibleDynamicPrimitives[MeshIdx];
		FPrimitiveViewRelevance& ViewRelevance = View.PrimitiveViewRelevanceMap[PrimitiveInfo->GetIndex()];
		if (ViewRelevance.GWData.bHair)
		{
			View.GWData.bHasHair = true;
			break;
		}
	}
}

void FHairWorksManager::UpdateHairFlagsAndClearRenderTargets(TArray<FViewInfo> &Views, FRHICommandList& RHICmdList)
{
	bool bHasHair = false;

	for (auto ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
	{
		auto& View = Views[ViewIndex];

		for (auto MeshIdx = 0; MeshIdx < View.VisibleDynamicPrimitives.Num(); ++MeshIdx)
		{
			auto PrimitiveInfo = View.VisibleDynamicPrimitives[MeshIdx];
			FPrimitiveViewRelevance& ViewRelevance = View.PrimitiveViewRelevanceMap[PrimitiveInfo->GetIndex()];
			if (ViewRelevance.GWData.bHair)
			{
				View.GWData.bHasHair = true;
				bHasHair = true;
				break;
			}
		}
	}

	StepSimulation();


// 	// Clear hair render targets
// 	if (bHasHair && HairMask && HairDepthZ)
// 	{
// 		SetRenderTarget(RHICmdList, HairMask->GetRenderTargetItem().TargetableTexture, HairDepthZ->GetRenderTargetItem().TargetableTexture, ESimpleRenderTargetMode::EClearColorAndDepth);
// 	}

}

void FHairWorksManager::RenderTranslucency(const FViewInfo &View, FRHICommandList& RHICmdList)
{
// 	// Draw hairs in translucency pass
// 	if (View.GWData.bHasHair)
// 	{
// //		UE_LOG(LogHairWorks, Log, TEXT("HM:RenderTranslucency"));
// 
// 
// 		for (auto MeshIdx = 0; MeshIdx < View.VisibleDynamicPrimitives.Num(); ++MeshIdx)
// 		{
// 			//TODO: Fix these stupid autos
// 			auto& PrimitiveInfo = *View.VisibleDynamicPrimitives[MeshIdx];
// 			auto& ViewRelevance = View.PrimitiveViewRelevanceMap[PrimitiveInfo.GetIndex()];
// 			if (!ViewRelevance.GWData.bHair)
// 				continue;
// 
// 			// Draw hair
// 			auto& HairSceneProxy = static_cast<FHairWorksSceneProxy&>(*PrimitiveInfo.Proxy);
// 			FVector4 IndirectLight[sizeof(FSHVectorRGB2) / sizeof(FVector4)] = { FVector4(0, 0, 0, 0), FVector4(0, 0, 0, 0), FVector4(0, 0, 0, 0) };
// 			if (PrimitiveInfo.IndirectLightingCacheAllocation
// 				&& PrimitiveInfo.IndirectLightingCacheAllocation->IsValid()
// 				&& View.Family->EngineShowFlags.GlobalIllumination)
// 			{
// 				const FIndirectLightingCacheAllocation& LightingAllocation = *PrimitiveInfo.IndirectLightingCacheAllocation;
// 				IndirectLight[0] = LightingAllocation.SingleSamplePacked[0];
// 				IndirectLight[1] = LightingAllocation.SingleSamplePacked[1];
// 				IndirectLight[2] = LightingAllocation.SingleSamplePacked[2];
// 			}
// 
// 			FSceneRenderTargets& SceneContext = FSceneRenderTargets::Get(RHICmdList);
// 
// 			auto LightAttenuationTexture = HairLightAttenuation && bLightShadowed ? HairLightAttenuation->GetRenderTargetItem().ShaderResourceTexture : nullptr;
// //			auto LightAttenuationTexture = HairLightAttenuation && bLightShadowed ? HairLightAttenuation->GetRenderTargetItem().ShaderResourceTexture : nullptr;
// 
// 			HairSceneProxy.DrawTranslucency(
// 				View,
// 				LightDirection,
// 				LightColor,
// 				LightAttenuationTexture,
// 				IndirectLight
// 				);
// 		}
// 	}

}

void FHairWorksManager::RenderToGBuffers(const FViewInfo& View, FRHICommandList& RHICmdList)
{
	if (View.GWData.bHasHair)
	{
		// NOTE: No messing with RTs! We're rendering to UE4s GBuffers!

		for (auto MeshIdx = 0; MeshIdx < View.VisibleDynamicPrimitives.Num(); ++MeshIdx)
		{
			auto PrimitiveInfo = View.VisibleDynamicPrimitives[MeshIdx];
			auto ViewRelevance = View.PrimitiveViewRelevanceMap[PrimitiveInfo->GetIndex()];
			if (!ViewRelevance.GWData.bHair)
				continue;

			FHairWorksSceneProxy* HairProxy = static_cast<FHairWorksSceneProxy*>(PrimitiveInfo->Proxy);

			HairProxy->DrawToGBuffers(View);
		}

	}
}

void FHairWorksManager::RenderBasePassDynamic(const FViewInfo& View, FRHICommandList& RHICmdList)
{
	if (View.GWData.bHasHair)
	{
		for (auto MeshIdx = 0; MeshIdx < View.VisibleDynamicPrimitives.Num(); ++MeshIdx)
		{
			auto PrimitiveInfo = View.VisibleDynamicPrimitives[MeshIdx];
			auto ViewRelevance = View.PrimitiveViewRelevanceMap[PrimitiveInfo->GetIndex()];
			if (!ViewRelevance.GWData.bHair)
				continue;

			FHairWorksSceneProxy* HairProxy = static_cast<FHairWorksSceneProxy*>(PrimitiveInfo->Proxy);

			HairProxy->DrawBasePass(View);
		}

	}

}

void FHairWorksManager::RenderProjectedShadows(FRHICommandList& RHICmdList, const FProjectedShadowInfo& ShadowInfo, const TArray<const FPrimitiveSceneInfo*, SceneRenderingAllocator> SubjectPrimitives, const FViewInfo *View)
{
	SCOPED_DRAW_EVENT(RHICmdList, RenderHairShadows);

	auto HairWorksSdk = GHairManager->GetHairworksSdk();

	for (auto PrimitiveIdx = 0; PrimitiveIdx < SubjectPrimitives.Num(); ++PrimitiveIdx)
	{
		auto PrimitiveInfo = SubjectPrimitives[PrimitiveIdx];
		auto ViewRelevance = View->PrimitiveViewRelevanceMap[PrimitiveInfo->GetIndex()];
		if (!ViewRelevance.GWData.bHair)
			continue;

		FHairWorksSceneProxy* HairProxy = static_cast<FHairWorksSceneProxy*>(PrimitiveInfo->Proxy);
		if (HairProxy->isHairInstanceNull())
			continue;

		// TODO: Is the hair casting shadows?

		// Setup render states and shaders
		TShaderMapRef<FScreenVS> VertexShader(GetGlobalShaderMap(ERHIFeatureLevel::SM5));

		if (ShadowInfo.CascadeSettings.bOnePassPointLightShadow)
		{
			// Setup camera
			const FBoxSphereBounds& PrimitiveBounds = HairProxy->GetBounds();

			FViewMatrices ViewMatrices[6];
			bool Visible[6];
			for (int32 FaceIndex = 0; FaceIndex < 6; FaceIndex++)
			{
				ViewMatrices[FaceIndex].ViewMatrix = ShadowInfo.OnePassShadowViewProjectionMatrices[FaceIndex];
				Visible[FaceIndex] = ShadowInfo.OnePassShadowFrustums[FaceIndex].IntersectBox(PrimitiveBounds.Origin, PrimitiveBounds.BoxExtent);
			}

			gfsdk_float4x4 HairViewMatrices[6];
			gfsdk_float4x4 HairProjMatrices[6];
			for (int FaceIdx = 0; FaceIdx < 6; ++FaceIdx)
			{
				HairViewMatrices[FaceIdx] = *(gfsdk_float4x4*)ViewMatrices[FaceIdx].ViewMatrix.M;
				HairProjMatrices[FaceIdx] = *(gfsdk_float4x4*)ViewMatrices[FaceIdx].ProjMatrix.M;
			}

			HairWorksSdk->SetViewProjectionForCubeMap(HairViewMatrices, HairProjMatrices, Visible, GFSDK_HAIR_LEFT_HANDED);

			// Setup shader
			static FGlobalBoundShaderState BoundShaderState;
			SetGlobalBoundShaderState(RHICmdList, ERHIFeatureLevel::SM5, BoundShaderState, GSimpleElementVertexDeclaration.VertexDeclarationRHI,
				*VertexShader, nullptr);
		}
		else
		{
			// Setup camera
			FViewMatrices ViewMatrices;
			ViewMatrices.ViewMatrix = FTranslationMatrix(ShadowInfo.PreShadowTranslation) * ShadowInfo.SubjectAndReceiverMatrix;
			HairWorksSdk->SetViewProjection((gfsdk_float4x4*)ViewMatrices.ViewMatrix.M, (gfsdk_float4x4*)ViewMatrices.ProjMatrix.M, GFSDK_HAIR_LEFT_HANDED);

			// Setup shader
			TShaderMapRef<FHairWorksShadowDepthPs> PixelShader(GetGlobalShaderMap(ERHIFeatureLevel::SM5));

			static FGlobalBoundShaderState BoundShaderState;
			SetGlobalBoundShaderState(RHICmdList, ERHIFeatureLevel::SM5, BoundShaderState, GSimpleElementVertexDeclaration.VertexDeclarationRHI,
				*VertexShader, *PixelShader);

 			SetShaderValue(RHICmdList, PixelShader->GetPixelShader(), PixelShader->ShadowParams, FVector2D(ShadowInfo.GetShaderDepthBias() * GHairManager->CVarHairShadowBiasScale.GetValueOnRenderThread(), ShadowInfo.InvMaxSubjectDepth));
		}

		// Flush render states
		RHICmdList.DrawPrimitive(0, 0, 0, 0);

		// Draw hair shadows
		HairProxy->DrawShadows(RHICmdList);
	}
}

void FHairWorksManager::RenderDepthDynamic(const FViewInfo* View, TArray<const FPrimitiveSceneInfo*, SceneRenderingAllocator> SubjectPrimitives, FViewMatrices ViewMatrices, float ShaderDepthBias, float InvMaxSubjectDepth)
{
//	UE_LOG(LogHairWorks, Log, TEXT("HM:RenderDepthDynamic"));

	//NB: Unlike all of the rest, this doesn't check to make sure the view has hair. Mistake?

	if (CVarHairShadows.GetValueOnRenderThread() == 1)
	{
		// Draw hairs.
		for (auto PrimitiveIdx = 0; PrimitiveIdx < SubjectPrimitives.Num(); ++PrimitiveIdx)
		{
			auto PrimitiveInfo = SubjectPrimitives[PrimitiveIdx];
			auto ViewRelevance = View->PrimitiveViewRelevanceMap[PrimitiveInfo->GetIndex()];
			if (!ViewRelevance.GWData.bHair)
				continue;

			FHairWorksSceneProxy* HairProxy = static_cast<FHairWorksSceneProxy*>(PrimitiveInfo->Proxy);

			HairProxy->DrawShadow(ViewMatrices, ShaderDepthBias, InvMaxSubjectDepth);
		}
	}
}

void FHairWorksManager::ClearHairLightAttenuation(FRHICommandList &RHICmdList)
{
	FSceneRenderTargets& SceneContext = FSceneRenderTargets::Get(RHICmdList);

	check(SceneContext.LightAttenuation.IsValid());

	// Swap in HairLightAttenuation
	SceneContext.LightAttenuation.Swap(HairLightAttenuation);

	// Use this to set render target
	SceneContext.BeginRenderingLightAttenuation(RHICmdList, false);

	// Clear the color, but not the depth or stencil
	RHICmdList.Clear(true, FLinearColor::White, false, 0, false, 0, FIntRect());

	// Swap back
	SceneContext.LightAttenuation.Swap(HairLightAttenuation);
}

void FHairWorksManager::UpdateViewPreShadow(const FProjectedShadowInfo &ShadowInfo, const FViewInfo &View, const TArray<const FPrimitiveSceneInfo*, SceneRenderingAllocator> &ReceiverPrimitives)
{
	if (View.GWData.bHasHair && CVarHairShadows.GetValueOnRenderThread()==1)
	{
		for (auto PrimitiveSceneInfo : ReceiverPrimitives)
		{
			auto ViewRelevance = View.PrimitiveViewRelevanceMap[PrimitiveSceneInfo->GetIndex()];
			if (ViewRelevance.GWData.bHair)
			{
				checkSlow(!ShadowInfo.GWData.bHairReceiver);
				ShadowInfo.GWData.bHairReceiver = true;
				break;
			}
		}
	}
}

void FHairWorksManager::RenderShadowProjection(const FProjectedShadowInfo& shadowInfo, const FViewInfo& View, FRHICommandList& RHICmdList)
{
	if (shadowInfo.GWData.bHairReceiver && CVarHairShadows.GetValueOnRenderThread()==1)
	{
		RHICmdList.SetDepthStencilState(TStaticDepthStencilState<false, CF_LessEqual>::GetRHI());

		FSceneRenderTargets& SceneContext = FSceneRenderTargets::Get(RHICmdList);

		check(SceneContext.LightAttenuation.IsValid());

// 		// Swap to replace render targets as well as shader resources.
// 		SceneContext.LightAttenuation.Swap(HairLightAttenuation);
// 		SceneContext.SceneDepthZ.Swap(HairDepthZ);
// 		SceneContext.BeginRenderingLightAttenuation(RHICmdList);

		RHICmdList.SetViewport(View.ViewRect.Min.X, View.ViewRect.Min.Y, 0.0f, View.ViewRect.Max.X, View.ViewRect.Max.Y, 1.0f);
	}

}

void FHairWorksManager::PostShadowRender(const FProjectedShadowInfo& shadowInfo, const FViewInfo& View, int32 ViewIndex, FRHICommandListImmediate& RHICmdList)
{
	if (shadowInfo.GWData.bHairReceiver && CVarHairShadows.GetValueOnRenderThread() == 1)
	{
		shadowInfo.GWData.bHairReceiver = false;

		FSceneRenderTargets& SceneContext = FSceneRenderTargets::Get(RHICmdList);

		check(SceneContext.LightAttenuation.IsValid());

// 		SceneContext.LightAttenuation.Swap(HairLightAttenuation);
// 		SceneContext.SceneDepthZ.Swap(HairDepthZ);
// 		SceneContext.BeginRenderingLightAttenuation(RHICmdList);

		RHICmdList.SetViewport(View.ViewRect.Min.X, View.ViewRect.Min.Y, 0.0f, View.ViewRect.Max.X, View.ViewRect.Max.Y, 1.0f);
	}
}

void FHairWorksManager::AfterRenderProjection(const FProjectedShadowInfo& shadowInfo, const FViewInfo& View, int32 ViewIndex, FRHICommandListImmediate& RHICmdList)
{
	if (View.GWData.bHasHair && !shadowInfo.bPreShadow && !shadowInfo.bSelfShadowOnly && CVarHairShadows.GetValueOnRenderThread() == 1)
	{
		FSceneRenderTargets& SceneContext = FSceneRenderTargets::Get(RHICmdList);

// 		check(SceneContext.LightAttenuation.IsValid());
// 
// 		SceneContext.SceneDepthZ.Swap(HairDepthZ);
// 		SceneContext.LightAttenuation.Swap(HairLightAttenuation);
//  		SceneContext.BeginRenderingLightAttenuation(RHICmdList);
//  		RHICmdList.SetViewport(View.ViewRect.Min.X, View.ViewRect.Min.Y, 0.0f, View.ViewRect.Max.X, View.ViewRect.Max.Y, 1.0f);

		checkSlow(!shadowInfo.GWData.bHairRenderProjection);
		shadowInfo.GWData.bHairRenderProjection = true;
		
		//JDM: Is this right? Check.
		//RenderProjection(FRHICommandListImmediate& RHICmdList, int32 ViewIndex, const class FViewInfo* View, bool bForwardShading) const;
		shadowInfo.RenderProjection(RHICmdList, ViewIndex, &View, false);
		shadowInfo.GWData.bHairRenderProjection = false;

// 		check(SceneContext.LightAttenuation.IsValid());
// 
// 		SceneContext.SceneDepthZ.Swap(HairDepthZ);
// 		SceneContext.LightAttenuation.Swap(HairLightAttenuation);
// 		SceneContext.BeginRenderingLightAttenuation(RHICmdList);
// 		RHICmdList.SetViewport(View.ViewRect.Min.X, View.ViewRect.Min.Y, 0.0f, View.ViewRect.Max.X, View.ViewRect.Max.Y, 1.0f);
	}
}

void FHairWorksManager::RenderVelocitiesInner(const FViewInfo &View)
{
// #ifdef HW_MSAA
// 
// 	bool bMSAA = CVarHairMsaa.GetValueOnRenderThread() == 1;
// 
// 	if (bMSAA)
// 		StartMsaa();
// #endif
// 
// 	for (auto PrimitiveIdx = 0; PrimitiveIdx < View.VisibleDynamicPrimitives.Num(); ++PrimitiveIdx)
// 	{
// 		auto PrimitiveInfo = View.VisibleDynamicPrimitives[PrimitiveIdx];
// 		auto ViewRelevance = View.PrimitiveViewRelevanceMap[PrimitiveInfo->GetIndex()];
// 		if (!ViewRelevance.GWData.bHair)
// 			continue;
// 
// 		// Draw hair
// 		FHairWorksSceneProxy* HairProxy = static_cast<FHairWorksSceneProxy*>(PrimitiveInfo->Proxy);
// 
// 		HairProxy->DrawVelocity(View, View.PrevViewMatrices);
// 	}
// 
// #ifdef HW_MSAA
// 	if (bMSAA)
// 	{
// 		FinishMsaa();
// 
// 		DrawPostColor(true);
// 	}
// #endif
}

void FHairWorksManager::SortVisibleDynamicPrimitives(FViewInfo &View)
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

/*
	There should be a better way to do this - we need a hook that tells this class it's safe to try to get the timer manager, as trying it after creation will give us an access violation.
*/
void FHairWorksManager::EnsureHairSetup()
{
// 	if (!SimTimerHandle.IsValid())
// 	{
// 		GWorld->GetTimerManager().SetTimer(SimTimerHandle, FTimerDelegate::CreateRaw(this, &FHairWorksManager::StepSimulation), 1.0f / 60.0f, true);
// 	}
}

#if WITH_EDITOR
void FHairWorksManager::UpdateHairInstanceDescriptor(GFSDK_HairInstanceID InstanceId, GFSDK_HairInstanceDescriptor& HairDesc)
{
#define HairVisualizationCVarUpdate(CVarName, MemberVarName)	\
	HairDesc.m_visualize##MemberVarName |= GHairManager->CVarHairVisualization##CVarName.GetValueOnRenderThread() != 0

	HairDesc.m_drawRenderHairs &= GHairManager->CVarHairVisualizationHair.GetValueOnRenderThread() != 0;
	HairVisualizationCVarUpdate(GuideCurves, GuideHairs);
	HairVisualizationCVarUpdate(SkinnedGuideCurves, SkinnedGuideHairs);
	HairVisualizationCVarUpdate(ControlPoints, ControlVertices);
	HairVisualizationCVarUpdate(GrowthMesh, GrowthMesh);
	HairVisualizationCVarUpdate(Bones, Bones);
	HairVisualizationCVarUpdate(BoundingBox, BoundingBox);
	HairVisualizationCVarUpdate(CollisionCapsules, Capsules);
	HairVisualizationCVarUpdate(HairInteraction, HairInteractions);
	HairVisualizationCVarUpdate(PinConstraints, PinConstraints);
	HairVisualizationCVarUpdate(ShadingNormal, ShadingNormals);
	HairVisualizationCVarUpdate(ShadingNormalCenter, ShadingNormalBone);

	HairWorksSdk->UpdateInstanceDescriptor(InstanceId, HairDesc);
#undef HairVisualizerCVarUpdate

}
#endif