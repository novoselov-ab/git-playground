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


FHairSceneProxy::FHairSceneProxy(const UPrimitiveComponent* InComponent, GFSDK_HairAssetID InAssetId):
FPrimitiveSceneProxy(InComponent),
AssetId(InAssetId),
HairInstanceId(GFSDK_HairInstanceID_NULL)
{

}

FHairSceneProxy::~FHairSceneProxy()
{
	if (HairInstanceId != GFSDK_HairInstanceID_NULL)
	{
		UE_LOG(LogHairWorks, Log, TEXT("Releasing Hair Instance %d"), static_cast<int32>(HairInstanceId));
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
	ShaderCacheSetting.SetFromInstanceDescriptor(CachedHairDescriptor);

	for (int i = 0; i < GFSDK_HAIR_NUM_TEXTURES; i++)
	{
		ShaderCacheSetting.isTextureUsed[i] = HairTextures[i] != nullptr;
	}

	GHairManager->GetHairworksSdk()->AddToShaderCache(ShaderCacheSetting);
}

void FHairSceneProxy::DrawToGBuffers(const FSceneView& View)
{
	if (HairInstanceId == GFSDK_HairInstanceID_NULL)
		return;

	// Update parameters
	CachedHairDescriptor.m_modelToWorld = (gfsdk_float4x4&)GetLocalToWorld().M;
	CachedHairDescriptor.m_useViewfrustrumCulling = false;

#if WITH_EDITOR

	// Take a copy since the CVars are going to mess with it.
	GFSDK_HairInstanceDescriptor HairDesc = CachedHairDescriptor;

	// Pass rendering parameters
	GHairManager->UpdateHairInstanceDescriptor(HairInstanceId, HairDesc);
#else
	GHairManager->GetHairworksSdk()->UpdateInstanceDescriptor(HairInstanceId, CachedHairDescriptor);	// Mainly for simulation.
#endif

	// Set states
	auto& RHICmdList = FRHICommandListExecutor::GetImmediateCommandList();

// 	//JDM:FIXME: Supposed to have depth write
// 	RHICmdList.SetDepthStencilState(TStaticDepthStencilState<false>::GetRHI());
// 
// 	RHICmdList.SetBlendState(TStaticBlendState<CW_RGBA, BO_Add, BF_SourceAlpha, BF_InverseSourceAlpha>::GetRHI());

	// Pass camera inforamtin
	auto ViewMatrices = View.ViewMatrices;


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
	PixelShader->SetParameters(RHICmdList, View, reinterpret_cast<GFSDK_Hair_ConstantBuffer&>(ConstBuffer), HairTextures);

	// To update shader states
	RHICmdList.DrawPrimitive(0, 0, 0, 0);

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

void FHairSceneProxy::DrawTranslucency(const FSceneView& View, const FVector& LightDir, const FLinearColor& LightColor, FTextureRHIRef LightAttenuation, const FVector4 IndirectLight[3])
{
	return;

// 	if (HairInstanceId == GFSDK_HairInstanceID_NULL)
// 		return;
// 
// 	// Update parameters
// 	CachedHairDescriptor.m_modelToWorld = (gfsdk_float4x4&)GetLocalToWorld().M;
// 	CachedHairDescriptor.m_useViewfrustrumCulling = false;
// 
// #if WITH_EDITOR
// 
// 	// Take a copy since the CVars are going to mess with it.
// 	GFSDK_HairInstanceDescriptor HairDesc = CachedHairDescriptor;
// 
// 	// Pass rendering parameters
// 	GHairManager->UpdateHairInstanceDescriptor(HairInstanceId, HairDesc);
// #else
// 	GHairManager->GetHairworksSdk()->UpdateInstanceDescriptor(HairInstanceId, CachedHairDescriptor);	// Mainly for simulation.
// #endif
// 
// 	// Set states
// 	auto& RHICmdList = FRHICommandListExecutor::GetImmediateCommandList();
// 
// 	//JDM:FIXME: Supposed to have depth write
// 	RHICmdList.SetDepthStencilState(TStaticDepthStencilState<false>::GetRHI());
// 
// 	RHICmdList.SetBlendState(TStaticBlendState<CW_RGBA, BO_Add, BF_SourceAlpha, BF_InverseSourceAlpha>::GetRHI());
// 
// 	// Pass camera inforamtin
// 	auto ViewMatrices = View.ViewMatrices;
// 
// 	
// 	auto HairWorksSdk = GHairManager->GetHairworksSdk();
// 
// 	HairWorksSdk->SetViewProjection((gfsdk_float4x4*)ViewMatrices.ViewMatrix.M, (gfsdk_float4x4*)ViewMatrices.ProjMatrix.M, GFSDK_HAIR_LEFT_HANDED);
// 
// 	// Setup shaders
// 	TShaderMapRef<FSimpleElementVS> VertexShader(GetGlobalShaderMap(ERHIFeatureLevel::SM5));
// 	TShaderMapRef<FHairWorksPs> PixelShader(GetGlobalShaderMap(ERHIFeatureLevel::SM5));
// 
// 	static FGlobalBoundShaderState BoundShaderState;
// 
// 	SetGlobalBoundShaderState(
// 		RHICmdList,
// 		ERHIFeatureLevel::SM5,
// 		BoundShaderState,
// 		GSimpleElementVertexDeclaration.VertexDeclarationRHI,
// 		*VertexShader,
// 		*PixelShader
// 		);
// 
// 	GFSDK_HairShaderConstantBuffer ConstBuffer;
// 	HairWorksSdk->PrepareShaderConstantBuffer(HairInstanceId, &ConstBuffer);
// 
// 	HairTextures.SetNum(GFSDK_HAIR_NUM_TEXTURES, false);
// 	PixelShader->SetParameters(RHICmdList, View, reinterpret_cast<GFSDK_Hair_ConstantBuffer&>(ConstBuffer), HairTextures, LightDir, LightColor, LightAttenuation, IndirectLight);
// 
// 	// To update shader states
// 	RHICmdList.DrawPrimitive(0, 0, 0, 0);
// 
// 	// Draw
// 	GFSDK_HairShaderSettings HairShaderSettings;
// 	HairShaderSettings.m_useCustomConstantBuffer = true;
// 
// 	ID3D11ShaderResourceView* HairSrvs[GFSDK_HAIR_NUM_SHADER_RESOUCES];
// 	HairWorksSdk->GetShaderResources(HairInstanceId, HairSrvs);
// 	auto& D3d11Rhi = static_cast<FD3D11DynamicRHI&>(*GDynamicRHI);
// 	D3d11Rhi.GetDeviceContext()->PSSetShaderResources(10, GFSDK_HAIR_NUM_SHADER_RESOUCES, HairSrvs);
// 
// 	HairWorksSdk->RenderHairs(HairInstanceId, &HairShaderSettings);
// 	HairWorksSdk->RenderVisualization(HairInstanceId);
}

void FHairSceneProxy::DrawShadows()
{
	SCOPED_DRAW_EVENT(RHICmdList, RenderHairShadow);

	auto HairWorksSdk = GHairManager->GetHairworksSdk();

	// Special for shadow
	GFSDK_HairInstanceDescriptor HairDesc;
	HairWorksSdk->CopyCurrentInstanceDescriptor(HairInstanceId, HairDesc);

	HairDesc.m_useBackfaceCulling = false;

	HairDesc.m_modelToWorld = (gfsdk_float4x4&)GetLocalToWorld().M;
	HairDesc.m_useViewfrustrumCulling = false;


	HairWorksSdk->UpdateInstanceDescriptor(HairInstanceId, HairDesc);

	// Handle shader cache.
	GFSDK_HairShaderCacheSettings ShaderCacheSetting;
	ShaderCacheSetting.SetFromInstanceDescriptor(HairDesc);
	check(HairTextures.Num() == GFSDK_HAIR_NUM_TEXTURES);
	for (int i = 0; i < GFSDK_HAIR_NUM_TEXTURES; i++)
	{
		ShaderCacheSetting.isTextureUsed[i] = (HairTextures[i] != nullptr);
	}

	HairWorksSdk->AddToShaderCache(ShaderCacheSetting);

	// Draw
	GFSDK_HairShaderSettings HairShaderSettings;
	HairShaderSettings.m_useCustomConstantBuffer = true;
	HairShaderSettings.m_shadowPass = true;

	HairWorksSdk->RenderHairs(HairInstanceId, &HairShaderSettings);
}

void FHairSceneProxy::DrawShadow(const FViewMatrices& ViewMatrices, float DepthBias, float DepthScale)
{
	if (HairInstanceId == GFSDK_HairInstanceID_NULL)
		return;

	auto HairWorksSdk = GHairManager->GetHairworksSdk();

	// JDM: GROSS! - GET RID OF THIS. Need another member in the config that encapsulates this, since the shader already knows it's a shadow render.
	GFSDK_HairInstanceDescriptor HairDesc = CachedHairDescriptor;

	HairDesc.m_width *= GHairManager->CVarHairShadowWidthScale.GetValueOnRenderThread();
	HairDesc.m_useBackfaceCulling = false;

	// Update parameters
	HairDesc.m_modelToWorld = (gfsdk_float4x4&)GetLocalToWorld().M;
	HairDesc.m_useViewfrustrumCulling = false;

	HairWorksSdk->UpdateInstanceDescriptor(HairInstanceId, HairDesc);	// Mainly for simulation.


	
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

// 	// Handle shader cache
 	UpdateShaderCache();

	// Draw
	GFSDK_HairShaderSettings HairShaderSettings;
	HairShaderSettings.m_useCustomConstantBuffer = true;
	HairShaderSettings.m_shadowPass = true;

	HairWorksSdk->RenderHairs(HairInstanceId, &HairShaderSettings);
}

void FHairSceneProxy::DrawBasePass(const FSceneView& View)
{
	return;

// 	if (HairInstanceId == GFSDK_HairInstanceID_NULL)
// 		return;
// 
// 	auto HairWorksSdk = GHairManager->GetHairworksSdk();
// 	
// 	// Update parameters
// 	CachedHairDescriptor.m_modelToWorld = (gfsdk_float4x4&)GetLocalToWorld().M;
// 	CachedHairDescriptor.m_useViewfrustrumCulling = false;
// 
// 	HairWorksSdk->UpdateInstanceDescriptor(HairInstanceId, CachedHairDescriptor);	// Mainly for simulation.
// 
// 	// Pass camera inforamtin
// 	auto ViewMatrices = View.ViewMatrices;
// 
// 	// Remove temporal AA jitter.
// 	if (!GHairManager->CVarHairTemporalAa.GetValueOnRenderThread())
// 	{
// 		ViewMatrices.ProjMatrix.M[2][0] = 0.0f;
// 		ViewMatrices.ProjMatrix.M[2][1] = 0.0f;
// 	}
// 
// 
// 	HairWorksSdk->SetViewProjection((gfsdk_float4x4*)ViewMatrices.ViewMatrix.M, (gfsdk_float4x4*)ViewMatrices.ProjMatrix.M, GFSDK_HAIR_LEFT_HANDED);
// 
// 	// Set render states
// 	auto& RHICmdList = FRHICommandListExecutor::GetImmediateCommandList();
// 
// 	RHICmdList.SetBlendState(TStaticBlendState<>::GetRHI());
// 	RHICmdList.SetDepthStencilState(TStaticDepthStencilState<>::GetRHI());
// 
// 	// Set shaders
// 	TShaderMapRef<FSimpleElementVS> VertexShader(GetGlobalShaderMap(ERHIFeatureLevel::SM5));
// 	TShaderMapRef<FHairWorksSimplePs> PixelShader(GetGlobalShaderMap(ERHIFeatureLevel::SM5));
// 	static FGlobalBoundShaderState BoundShaderState;
// 	SetGlobalBoundShaderState(RHICmdList, ERHIFeatureLevel::SM5, BoundShaderState, GSimpleElementVertexDeclaration.VertexDeclarationRHI,
// 		*VertexShader, *PixelShader);
// 
// 	// Draw
// 	GFSDK_HairShaderSettings HairShaderSettings;
// 	HairShaderSettings.m_useCustomConstantBuffer = true;
// 
// 	HairWorksSdk->RenderHairs(HairInstanceId, &HairShaderSettings);
}

void FHairSceneProxy::DrawVelocity(const FSceneView& View, const FViewMatrices& PrevViewMatrices)
{
	return;

// 	// Remove temporal AA jitter.
// 	if(!GHairManager->CVarHairTemporalAa.GetValueOnRenderThread())
// 		return;
// 
// 	if (!GHairManager->CVarHairOutputVelocity.GetValueOnRenderThread())
// 		return;
// 
// 	if(HairInstanceId == GFSDK_HairInstanceID_NULL)
// 		return;
// 
// 	auto HairWorksSdk = GHairManager->GetHairworksSdk();
// 
// 	// Update parameters
// 	CachedHairDescriptor.m_modelToWorld = (gfsdk_float4x4&)GetLocalToWorld().M;
// 	CachedHairDescriptor.m_useViewfrustrumCulling = false;
// 
// 	HairWorksSdk->UpdateInstanceDescriptor(HairInstanceId, CachedHairDescriptor);	// Mainly for simulation.
// 
// 
// 	// Pass camera inforamtin
// 	const auto& ViewMatrices = View.ViewMatrices;
// 	HairWorksSdk->SetViewProjection((gfsdk_float4x4*)ViewMatrices.ViewMatrix.M, (gfsdk_float4x4*)ViewMatrices.ProjMatrix.M, GFSDK_HAIR_LEFT_HANDED);
// 	HairWorksSdk->SetPrevViewProjection((gfsdk_float4x4*)PrevViewMatrices.ViewMatrix.M, (gfsdk_float4x4*)PrevViewMatrices.ProjMatrix.M, GFSDK_HAIR_LEFT_HANDED);
// 
// 	// Set render states
// 	auto& RHICmdList = FRHICommandListExecutor::GetImmediateCommandList();
// 
// 	RHICmdList.SetBlendState(TStaticBlendState<CW_NONE>::GetRHI());
// 	RHICmdList.SetBlendState(TStaticBlendState<>::GetRHI());
// 
// 	//JDM:FIXME Supposed to have depth write...
// 	RHICmdList.SetDepthStencilState(TStaticDepthStencilState<false>::GetRHI());
// 
// 	// Set shaders
// 	TShaderMapRef<FSimpleElementVS> VertexShader(GetGlobalShaderMap(ERHIFeatureLevel::SM5));
// 	TShaderMapRef<FHairWorksVelocityPs> PixelShader(GetGlobalShaderMap(ERHIFeatureLevel::SM5));
// 	static FGlobalBoundShaderState BoundShaderState;
// 	SetGlobalBoundShaderState(RHICmdList, ERHIFeatureLevel::SM5, BoundShaderState, GSimpleElementVertexDeclaration.VertexDeclarationRHI,
// 		*VertexShader, *PixelShader);
// 
// 	GFSDK_HairShaderConstantBuffer ConstBuffer;
// 	HairWorksSdk->PrepareShaderConstantBuffer(HairInstanceId, &ConstBuffer);
// 	SetShaderValue(RHICmdList, PixelShader->GetPixelShader(), PixelShader->HairConstantBuffer, reinterpret_cast<GFSDK_Hair_ConstantBuffer&>(ConstBuffer));
// 
// 	// To update shader states
// 	RHICmdList.DrawPrimitive(0, 0, 0, 0);
// 
// 	// Draw
// 	GFSDK_HairShaderSettings HairShaderSettings;
// 	HairShaderSettings.m_useCustomConstantBuffer = true;
// 
// 	ID3D11ShaderResourceView* HairSrvs[GFSDK_HAIR_NUM_SHADER_RESOUCES];
// 	HairWorksSdk->GetShaderResources(HairInstanceId, HairSrvs);
// 	auto& D3d11Rhi = static_cast<FD3D11DynamicRHI&>(*GDynamicRHI);
// 	D3d11Rhi.GetDeviceContext()->PSSetShaderResources(10, GFSDK_HAIR_NUM_SHADER_RESOUCES, HairSrvs);
// 
// 	HairWorksSdk->RenderHairs(HairInstanceId, &HairShaderSettings);
}

void FHairSceneProxy::GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, class FMeshElementCollector& Collector) const
{
	//UE_LOG(LogHairWorks, Log, TEXT("HSP: GetDynamicMeshElements"));
}

void FHairSceneProxy::CreateRenderThreadResources()
{
	FPrimitiveSceneProxy::CreateRenderThreadResources();

	auto HairWorksSdk = GHairManager->GetHairworksSdk();

	check(HairWorksSdk);

	UE_LOG(LogHairWorks, Log, TEXT("FHairSceneProxy::CreateRenderThreadResources"));

	check(AssetId != GFSDK_HairAssetID_NULL);

	HairWorksSdk->CreateHairInstance(AssetId, &HairInstanceId);

	UE_LOG(LogHairWorks, Log, TEXT("Created new hair instance %d"), static_cast<int32>(HairInstanceId));
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
	HairWorksSdk->GetNumBones(AssetId, &BoneNum);

	UE_LOG(LogHairWorks, Log, TEXT("SetupBoneMapping: Bone count %d, asset %d"), BoneNum, static_cast<int32>(AssetId));

	BoneMapping.SetNumUninitialized(BoneNum);

	for (auto Idx = 0; Idx < BoneMapping.Num(); ++Idx)
	{
		gfsdk_char BoneName[GFSDK_HAIR_MAX_STRING];
		HairWorksSdk->GetBoneName(AssetId, Idx, BoneName);

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
	TArray<FMatrix> BoneMatrices;
	
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
			ThisProxy.UpdateHairParams(HairDesc, HairTextures);
		}
	);
}


void FHairSceneProxy::UpdateHairParams(GFSDK_HairInstanceDescriptor& InHairDesc, const TArray<FTexture2DRHIRef>& InHairTextures)
{
	if (HairInstanceId == GFSDK_HairInstanceID_NULL)
		return;

	// Take a local copy of these
	CachedHairDescriptor = InHairDesc;
	HairTextures = InHairTextures;

	auto HairWorksSdk = GHairManager->GetHairworksSdk();

	// Update parameters
	CachedHairDescriptor.m_modelToWorld = (gfsdk_float4x4&)GetLocalToWorld().M;
	CachedHairDescriptor.m_useViewfrustrumCulling = false;

	HairWorksSdk->UpdateInstanceDescriptor(HairInstanceId, CachedHairDescriptor);	// Mainly for simulation.

	// Update textures
	check(HairTextures.Num() == GFSDK_HAIR_NUM_TEXTURES);

	for (auto Idx = 0; Idx < GFSDK_HAIR_NUM_TEXTURES; ++Idx)
	{
		auto TextureRef = HairTextures[Idx];
		auto Texture = static_cast<TD3D11Texture2D<FD3D11BaseTexture2D>*>(TextureRef.GetReference());
		HairWorksSdk->SetTextureSRV(HairInstanceId, (GFSDK_HAIR_TEXTURE_TYPE)Idx, Texture ? Texture->GetShaderResourceView() : nullptr);
	}

	UpdateShaderCache();
}


bool FHairSceneProxy::GetHairBounds_GameThread(FBoxSphereBounds& Bounds)const
{
	if (HairInstanceId == GFSDK_HairInstanceID_NULL)
		return false;

	auto HairWorksSdk = GHairManager->GetHairworksSdk();

	gfsdk_float3 Min;
	gfsdk_float3 Max;

	if(HairWorksSdk->GetBounds(HairInstanceId, &Min, &Max) != GFSDK_HAIR_RETURN_OK)
		return false;

	Bounds = FBoxSphereBounds(FBox(FVector(Min.x, Min.y, Min.z), FVector(Max.x, Max.y, Max.z)));

	return true;
}
