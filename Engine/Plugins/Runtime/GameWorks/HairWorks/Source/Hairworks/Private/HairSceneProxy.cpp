#include "HairworksPCH.h"

#include "D3D11RHIPrivate.h"	// Hack
#include "RHIStaticStates.h"
#include "SkeletalRenderGPUSkin.h"
#include "SimpleElementShaders.h"
#include "RendererPrivate.h"
#include "SceneRendering.h"
#include "PrimitiveSceneInfo.h"
#include "PostProcess/SceneRenderTargets.h"
#include "HairShaders.h"

FHairSceneProxy::FHairSceneProxy(const UPrimitiveComponent* InComponent, UHair* InHair)
	:FPrimitiveSceneProxy(InComponent)
	, Hair(InHair)
{
}

FHairSceneProxy::~FHairSceneProxy()
{
	if (HairInstanceId != GFSDK_HairInstanceID_NULL)
	{
		UE_LOG(LogHairWorks, Log, TEXT("Releasing Hair Instance"));
		GHairManager->GetHairworksSdk()->FreeHairInstance(HairInstanceId);
	}
}

uint32 FHairSceneProxy::GetMemoryFootprint(void) const
{
	return 0;
}


// JDM: TODO: Call this less. It seems like it doesn't do anything after the first time anyway. Investigate more.
void FHairSceneProxy::UpdateShaderCache()
{
	GFSDK_HairShaderCacheSettings ShaderCacheSetting;
	ShaderCacheSetting.SetFromInstanceDescriptor(HairDesc);
	HairTextures.SetNum(GFSDK_HAIR_NUM_TEXTURES, false);
	for (int i = 0; i < GFSDK_HAIR_NUM_TEXTURES; i++)
	{
		ShaderCacheSetting.isTextureUsed[i] = HairTextures[i] != nullptr;
	}

	GHairManager->GetHairworksSdk()->AddToShaderCache(ShaderCacheSetting);
}

void FHairSceneProxy::DrawTranslucency(const FSceneView& View, const FVector& LightDir, const FLinearColor& LightColor, FTextureRHIRef LightAttenuation, const FVector4 IndirectLight[3])
{
	if (HairInstanceId == GFSDK_HairInstanceID_NULL)
		return;

	//UE_LOG(LogHairWorks, Log, TEXT("DrawTranslucency"));

	// Simulate.
	GHairManager->StepSimulation();

	// Pass rendering parameters
	GHairManager->UpdateHairInstanceDescriptor(HairInstanceId, HairDesc);

	// Set states
	auto& RHICmdList = FRHICommandListExecutor::GetImmediateCommandList();

	RHICmdList.SetDepthStencilState(TStaticDepthStencilState<false>::GetRHI());

	RHICmdList.SetBlendState(TStaticBlendState<CW_RGBA, BO_Add, BF_SourceAlpha, BF_InverseSourceAlpha>::GetRHI());

	// Pass camera inforamtin
	auto ViewMatrices = View.ViewMatrices;

	// Remove temporal AA jitter.
	if (!GHairManager->CVarHairTemporalAa.GetValueOnRenderThread())
	{
		ViewMatrices.ProjMatrix.M[2][0] = 0.0f;
		ViewMatrices.ProjMatrix.M[2][1] = 0.0f;
	}
	
	auto HairWorksSdk = GHairManager->GetHairworksSdk();

	HairWorksSdk->SetViewProjection((gfsdk_float4x4*)ViewMatrices.ViewMatrix.M, (gfsdk_float4x4*)ViewMatrices.ProjMatrix.M, GFSDK_HAIR_LEFT_HANDED);

	// Setup shaders
	TShaderMapRef<FSimpleElementVS> VertexShader(GetGlobalShaderMap(ERHIFeatureLevel::SM5));
	TShaderMapRef<FHairWorksPs> PixelShader(GetGlobalShaderMap(ERHIFeatureLevel::SM5));

	static FGlobalBoundShaderState BoundShaderState;

	SetGlobalBoundShaderState(
		RHICmdList,
		ERHIFeatureLevel::SM5,
		BoundShaderState,
		GSimpleElementVertexDeclaration.VertexDeclarationRHI,
		*VertexShader,
		*PixelShader
		);

	GFSDK_HairShaderConstantBuffer ConstBuffer;
	HairWorksSdk->PrepareShaderConstantBuffer(HairInstanceId, &ConstBuffer);

	HairTextures.SetNum(GFSDK_HAIR_NUM_TEXTURES, false);
	PixelShader->SetParameters(RHICmdList, View, reinterpret_cast<GFSDK_Hair_ConstantBuffer&>(ConstBuffer), HairTextures, LightDir, LightColor, LightAttenuation, IndirectLight);

	// To update shader states
	RHICmdList.DrawPrimitive(0, 0, 0, 0);

	// Handle shader cache.
	UpdateShaderCache();

	// Draw
	GFSDK_HairShaderSettings HairShaderSettings;
	HairShaderSettings.m_useCustomConstantBuffer = true;

	ID3D11ShaderResourceView* HairSrvs[GFSDK_HAIR_NUM_SHADER_RESOUCES];
	HairWorksSdk->GetShaderResources(HairInstanceId, HairSrvs);
	auto& D3d11Rhi = static_cast<FD3D11DynamicRHI&>(*GDynamicRHI);
	D3d11Rhi.GetDeviceContext()->PSSetShaderResources(10, GFSDK_HAIR_NUM_SHADER_RESOUCES, HairSrvs);

	HairWorksSdk->RenderHairs(HairInstanceId, &HairShaderSettings);
	HairWorksSdk->RenderVisualization(HairInstanceId);
}

void FHairSceneProxy::DrawShadow(const FViewMatrices& ViewMatrices, float DepthBias, float DepthScale)
{
	if (HairInstanceId == GFSDK_HairInstanceID_NULL)
		return;

	//UE_LOG(LogHairWorks, Log, TEXT("DrawShadow"));

	// Simulate
	GHairManager->StepSimulation();

	auto HairWorksSdk = GHairManager->GetHairworksSdk();
	
	// Pass rendering parameters
	HairDesc.m_castShadows = true;

	const auto bOrgUseBackFaceCulling = HairDesc.m_useBackfaceCulling;
	const auto OrgHairWidth = HairDesc.m_width;
	HairDesc.m_width *= GHairManager->CVarHairShadowWidthScale.GetValueOnRenderThread();
	HairDesc.m_useBackfaceCulling = false;

	HairWorksSdk->UpdateInstanceDescriptor(HairInstanceId, HairDesc);

	// Revert parameters
	HairDesc.m_width = OrgHairWidth;
	HairDesc.m_useBackfaceCulling = bOrgUseBackFaceCulling;


	// Pass camera inforamtin
	HairWorksSdk->SetViewProjection((gfsdk_float4x4*)ViewMatrices.ViewMatrix.M, (gfsdk_float4x4*)ViewMatrices.ProjMatrix.M, GFSDK_HAIR_LEFT_HANDED);

	// Set shaders.
	auto& RHICmdList = FRHICommandListExecutor::GetImmediateCommandList();

	TShaderMapRef<FSimpleElementVS> VertexShader(GetGlobalShaderMap(ERHIFeatureLevel::SM5));
	TShaderMapRef<FHairWorksShadowDepthPs> PixelShader(GetGlobalShaderMap(ERHIFeatureLevel::SM5));

	static FGlobalBoundShaderState BoundShaderState;
	SetGlobalBoundShaderState(RHICmdList, ERHIFeatureLevel::SM5, BoundShaderState, GSimpleElementVertexDeclaration.VertexDeclarationRHI,
		*VertexShader, *PixelShader);

	SetShaderValue(RHICmdList, PixelShader->GetPixelShader(), PixelShader->ShadowParams, FVector2D(DepthBias * GHairManager->CVarHairShadowBiasScale.GetValueOnRenderThread(), DepthScale));

	// To update shader states
	RHICmdList.DrawPrimitive(0, 0, 0, 0);

	// Handle shader cache
	UpdateShaderCache();

	// Draw
	GFSDK_HairShaderSettings HairShaderSettings;
	HairShaderSettings.m_useCustomConstantBuffer = true;
	HairShaderSettings.m_shadowPass = true;

	HairWorksSdk->RenderHairs(HairInstanceId, &HairShaderSettings);
}

void FHairSceneProxy::DrawBasePass(const FSceneView& View)
{
	if (HairInstanceId == GFSDK_HairInstanceID_NULL)
		return;

	//UE_LOG(LogHairWorks, Log, TEXT("DrawBasePass"));

	// Simulate
	GHairManager->StepSimulation();

	// Pass camera inforamtin
	auto ViewMatrices = View.ViewMatrices;

	// Remove temporal AA jitter.
	if (!GHairManager->CVarHairTemporalAa.GetValueOnRenderThread())
	{
		ViewMatrices.ProjMatrix.M[2][0] = 0.0f;
		ViewMatrices.ProjMatrix.M[2][1] = 0.0f;
	}

	auto HairWorksSdk = GHairManager->GetHairworksSdk();

	HairWorksSdk->SetViewProjection((gfsdk_float4x4*)ViewMatrices.ViewMatrix.M, (gfsdk_float4x4*)ViewMatrices.ProjMatrix.M, GFSDK_HAIR_LEFT_HANDED);

	// Set render states
	auto& RHICmdList = FRHICommandListExecutor::GetImmediateCommandList();

	RHICmdList.SetBlendState(TStaticBlendState<>::GetRHI());
	RHICmdList.SetDepthStencilState(TStaticDepthStencilState<>::GetRHI());

	// Set shaders
	TShaderMapRef<FSimpleElementVS> VertexShader(GetGlobalShaderMap(ERHIFeatureLevel::SM5));
	TShaderMapRef<FHairWorksSimplePs> PixelShader(GetGlobalShaderMap(ERHIFeatureLevel::SM5));
	static FGlobalBoundShaderState BoundShaderState;
	SetGlobalBoundShaderState(RHICmdList, ERHIFeatureLevel::SM5, BoundShaderState, GSimpleElementVertexDeclaration.VertexDeclarationRHI,
		*VertexShader, *PixelShader);

	// Handle shader cache.
	UpdateShaderCache();

	// Draw
	GFSDK_HairShaderSettings HairShaderSettings;
	HairShaderSettings.m_useCustomConstantBuffer = true;

	HairWorksSdk->RenderHairs(HairInstanceId, &HairShaderSettings);
}

void FHairSceneProxy::DrawVelocity(const FSceneView& View, const FViewMatrices& PrevViewMatrices)
{
	// Remove temporal AA jitter.
	if(!GHairManager->CVarHairTemporalAa.GetValueOnRenderThread())
		return;

	if (!GHairManager->CVarHairOutputVelocity.GetValueOnRenderThread())
		return;

	if(HairInstanceId == GFSDK_HairInstanceID_NULL)
		return;

//	UE_LOG(LogHairWorks, Log, TEXT("DrawVelocity"));


	// Simulate
	GHairManager->StepSimulation();

	auto HairWorksSdk = GHairManager->GetHairworksSdk();

	// Pass camera inforamtin
	const auto& ViewMatrices = View.ViewMatrices;
	HairWorksSdk->SetViewProjection((gfsdk_float4x4*)ViewMatrices.ViewMatrix.M, (gfsdk_float4x4*)ViewMatrices.ProjMatrix.M, GFSDK_HAIR_LEFT_HANDED);
	HairWorksSdk->SetPrevViewProjection((gfsdk_float4x4*)PrevViewMatrices.ViewMatrix.M, (gfsdk_float4x4*)PrevViewMatrices.ProjMatrix.M, GFSDK_HAIR_LEFT_HANDED);

	// Set render states
	auto& RHICmdList = FRHICommandListExecutor::GetImmediateCommandList();

	RHICmdList.SetBlendState(TStaticBlendState<CW_NONE>::GetRHI());
	RHICmdList.SetBlendState(TStaticBlendState<>::GetRHI());
	RHICmdList.SetDepthStencilState(TStaticDepthStencilState<false>::GetRHI());

	// Set shaders
	TShaderMapRef<FSimpleElementVS> VertexShader(GetGlobalShaderMap(ERHIFeatureLevel::SM5));
	TShaderMapRef<FHairWorksVelocityPs> PixelShader(GetGlobalShaderMap(ERHIFeatureLevel::SM5));
	static FGlobalBoundShaderState BoundShaderState;
	SetGlobalBoundShaderState(RHICmdList, ERHIFeatureLevel::SM5, BoundShaderState, GSimpleElementVertexDeclaration.VertexDeclarationRHI,
		*VertexShader, *PixelShader);

	GFSDK_HairShaderConstantBuffer ConstBuffer;
	HairWorksSdk->PrepareShaderConstantBuffer(HairInstanceId, &ConstBuffer);
	SetShaderValue(RHICmdList, PixelShader->GetPixelShader(), PixelShader->HairConstantBuffer, reinterpret_cast<GFSDK_Hair_ConstantBuffer&>(ConstBuffer));

	// To update shader states
	RHICmdList.DrawPrimitive(0, 0, 0, 0);

	// Handle shader cache.
	UpdateShaderCache();

	// Draw
	GFSDK_HairShaderSettings HairShaderSettings;
	HairShaderSettings.m_useCustomConstantBuffer = true;

	ID3D11ShaderResourceView* HairSrvs[GFSDK_HAIR_NUM_SHADER_RESOUCES];
	HairWorksSdk->GetShaderResources(HairInstanceId, HairSrvs);
	auto& D3d11Rhi = static_cast<FD3D11DynamicRHI&>(*GDynamicRHI);
	D3d11Rhi.GetDeviceContext()->PSSetShaderResources(10, GFSDK_HAIR_NUM_SHADER_RESOUCES, HairSrvs);

	HairWorksSdk->RenderHairs(HairInstanceId, &HairShaderSettings);
}

void FHairSceneProxy::GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, class FMeshElementCollector& Collector) const
{
	//UE_LOG(LogHairWorks, Log, TEXT("HSP: GetDynamicMeshElements"));
}

void FHairSceneProxy::CreateRenderThreadResources()
{
	FPrimitiveSceneProxy::CreateRenderThreadResources();

	// JDM: Do not do this in the render thread unless we have to!
	// JDM: This is pretty heavy! Shouldn't be here. Reuse this data from somewhere else.
	auto HairWorksSdk = GHairManager->GetHairworksSdk();

	check(HairWorksSdk);

	UE_LOG(LogHairWorks, Log, TEXT("CreateRenderThreadResources"));

	// Initialize Hair asset and instance
	if (Hair->AssetId == GFSDK_HairAssetID_NULL)
	{
		GFSDK_HairConversionSettings LoadSettings;
		LoadSettings.m_targetHandednessHint = GFSDK_HAIR_HANDEDNESS_HINT::GFSDK_HAIR_LEFT_HANDED;
		LoadSettings.m_targetUpAxisHint = GFSDK_HAIR_UP_AXIS_HINT::GFSDK_HAIR_Z_UP;
		HairWorksSdk->LoadHairAssetFromMemory(Hair->AssetData.GetData(), Hair->AssetData.Num(), &Hair->AssetId, 0, &LoadSettings);
	}


	HairWorksSdk->CreateHairInstance(Hair->AssetId, &HairInstanceId);

	UE_LOG(LogHairWorks, Log, TEXT("Created new hair instance %d"), static_cast<int32>(HairInstanceId));

	// Get parameters
	HairWorksSdk->CopyInstanceDescriptorFromAsset(Hair->AssetId, HairDesc);
}

FPrimitiveViewRelevance FHairSceneProxy::GetViewRelevance(const FSceneView* View)
{
	FPrimitiveViewRelevance ViewRel;
	ViewRel.bDrawRelevance = IsShown(View);
	ViewRel.bShadowRelevance = IsShadowCast(View);
	ViewRel.bDynamicRelevance = true;
	ViewRel.bRenderCustomDepth = false;
	ViewRel.bRenderInMainPass = true;
	ViewRel.bOpaqueRelevance = true;
	ViewRel.bMaskedRelevance = false;
	ViewRel.bDistortionRelevance = false;
	ViewRel.bSeparateTranslucencyRelevance = false;
	ViewRel.bNormalTranslucencyRelevance = true;

	ViewRel.GWData.bHair = true;

	return ViewRel;
}

void FHairSceneProxy::SetupBoneMapping_GameThread(const TArray<FMeshBoneInfo>& Bones)
{
	// Send bone names to setup mapping
	ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
		HairSetupBoneMapping,
		FHairSceneProxy&, ThisProxy, *this,
		TArray<FMeshBoneInfo>, Bones, Bones,
		{
			ThisProxy.SetupBoneMapping_RenderThread(Bones);
		}
		)
}

void FHairSceneProxy::SetupBoneMapping_RenderThread(const TArray<FMeshBoneInfo>& Bones)
{
	// Setup bone mapping
	if (HairInstanceId == GFSDK_HairInstanceID_NULL)
		return;
	
	auto HairWorksSdk = GHairManager->GetHairworksSdk();

	gfsdk_U32 BoneNum = 0;
	HairWorksSdk->GetNumBones(Hair->AssetId, &BoneNum);

	BoneMapping.SetNumUninitialized(BoneNum);

	for (auto Idx = 0; Idx < BoneMapping.Num(); ++Idx)
	{
		gfsdk_char BoneName[GFSDK_HAIR_MAX_STRING];
		HairWorksSdk->GetBoneName(Hair->AssetId, Idx, BoneName);

		BoneMapping[Idx] = Bones.IndexOfByPredicate([&](const FMeshBoneInfo& BoneInfo){return BoneInfo.Name == BoneName; });
	}
}

void FHairSceneProxy::UpdateBones_GameThread(USkinnedMeshComponent& ParentSkeleton)
{
	// Send bone matrices for rendering
	TSharedRef<FDynamicSkelMeshObjectDataGPUSkin> DynamicData = MakeShareable(new FDynamicSkelMeshObjectDataGPUSkin(
		&ParentSkeleton,
		ParentSkeleton.SkeletalMesh->GetResourceForRendering(),
		0,
		TArray<FActiveVertexAnim>()
		));

	ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
		HairUpdateData,
		FHairSceneProxy&, ThisProxy, *this,
		TSharedRef<FDynamicSkelMeshObjectDataGPUSkin>, DynamicData, DynamicData,
		{
			ThisProxy.UpdateBones_RenderThread(DynamicData->ReferenceToLocal);
		}
		);
}

void FHairSceneProxy::UpdateBones_RenderThread(const TArray<FMatrix>& RefMatrices)
{
	if (HairInstanceId == GFSDK_HairInstanceID_NULL)
		return;

	// Setup bones
	FMemMark MemMark(FMemStack::Get());
	TArray<FMatrix, TMemStackAllocator<>> BoneMatrices;
	
	BoneMatrices.SetNumUninitialized(BoneMapping.Num());

	for (auto Idx = 0; Idx < BoneMatrices.Num(); ++Idx)
	{
		auto SkeletonBoneIdx = BoneMapping[Idx];
		BoneMatrices[Idx] = SkeletonBoneIdx >= 0 && SkeletonBoneIdx < RefMatrices.Num() ? RefMatrices[SkeletonBoneIdx] : FMatrix::Identity;
	}

	auto HairWorksSdk = GHairManager->GetHairworksSdk();

	// Update to hair
	HairWorksSdk->UpdateSkinningMatrices(HairInstanceId, BoneMatrices.Num(), (gfsdk_float4x4*)BoneMatrices.GetData());
}

void FHairSceneProxy::UpdateHairParams_GameThread(const GFSDK_HairInstanceDescriptor& HairDesc, const TArray<FTexture2DRHIRef>& HairTextures)
{
	ENQUEUE_UNIQUE_RENDER_COMMAND_THREEPARAMETER(
		HairUpdateParams,
		FHairSceneProxy&, ThisProxy, *this,
		GFSDK_HairInstanceDescriptor, HairDesc, HairDesc,
		const TArray<FTexture2DRHIRef>, HairTextures, HairTextures,
		{
			ThisProxy.UpdateHairParams_RenderThread(HairDesc, HairTextures);
		}
		);
}

void FHairSceneProxy::UpdateHairParams_RenderThread(const GFSDK_HairInstanceDescriptor& InHairDesc, const TArray<FTexture2DRHIRef>& InHairTextures)
{
	if (HairInstanceId == GFSDK_HairInstanceID_NULL)
		return;

	auto HairWorksSdk = GHairManager->GetHairworksSdk();

	// Update parameters
	HairDesc = InHairDesc;
	HairDesc.m_modelToWorld = (gfsdk_float4x4&)GetLocalToWorld().M;
	HairDesc.m_useViewfrustrumCulling = false;
	HairWorksSdk->UpdateInstanceDescriptor(HairInstanceId, HairDesc);	// Mainly for simulation.

	// Update textures
	checkSlow(InHairTextures.Num() >= GFSDK_HAIR_NUM_TEXTURES);
	HairTextures = InHairTextures;
	HairTextures.SetNum(GFSDK_HAIR_NUM_TEXTURES, false);

	for (auto Idx = 0; Idx < GFSDK_HAIR_NUM_TEXTURES; ++Idx)
	{
		auto TextureRef = HairTextures[Idx];
		auto* Texture = static_cast<TD3D11Texture2D<FD3D11BaseTexture2D>*>(TextureRef.GetReference());
		HairWorksSdk->SetTextureSRV(HairInstanceId, (GFSDK_HAIR_TEXTURE_TYPE)Idx, Texture ? Texture->GetShaderResourceView() : nullptr);
	}
}


bool FHairSceneProxy::GetHairBounds_GameThread(FBoxSphereBounds& Bounds)const
{
	if (HairInstanceId == GFSDK_HairInstanceID_NULL)
		return false;

	auto HairWorksSdk = GHairManager->GetHairworksSdk();

	FBox HairBox;
	if(HairWorksSdk->GetBounds(HairInstanceId, reinterpret_cast<gfsdk_float3*>(&HairBox.Min), reinterpret_cast<gfsdk_float3*>(&HairBox.Max)) != GFSDK_HAIR_RETURN_OK)
		return false;

	Bounds = FBoxSphereBounds(HairBox);

	return true;
}
