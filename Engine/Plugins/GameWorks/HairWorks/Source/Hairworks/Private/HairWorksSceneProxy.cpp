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


FHairWorksSceneProxy::FHairWorksSceneProxy(const UPrimitiveComponent* InComponent, GFSDK_HairAssetID InAssetId):
FPrimitiveSceneProxy(InComponent),
AssetId(InAssetId),
HairInstanceId(GFSDK_HairInstanceID_NULL)
{

}

FHairWorksSceneProxy::~FHairWorksSceneProxy()
{
	if (HairInstanceId != GFSDK_HairInstanceID_NULL)
	{
		UE_LOG(LogHairWorks, Log, TEXT("Releasing Hair Instance %d"), static_cast<int32>(HairInstanceId));
		GHairManager->GetHairworksSdk()->FreeHairInstance(HairInstanceId);
	}
}

uint32 FHairWorksSceneProxy::GetMemoryFootprint(void) const
{
	return 0;
}

void FHairWorksSceneProxy::DrawToGBuffers(const FSceneView& View)
{
	if (HairInstanceId == GFSDK_HairInstanceID_NULL)
		return;

	// Grab the hairdesc
	GFSDK_HairInstanceDescriptor CachedHairDescriptor;
	GHairManager->GetHairworksSdk()->CopyCurrentInstanceDescriptor(HairInstanceId, CachedHairDescriptor);

	// Update parameters
	CachedHairDescriptor.m_modelToWorld = (gfsdk_float4x4&)GetLocalToWorld().M;
	CachedHairDescriptor.m_useViewfrustrumCulling = false;

#if WITH_EDITOR
	// Pass rendering parameters
	GHairManager->UpdateHairInstanceDescriptor(HairInstanceId, CachedHairDescriptor);
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

void FHairWorksSceneProxy::DrawTranslucency(const FSceneView& View, const FVector& LightDir, const FLinearColor& LightColor, FTextureRHIRef LightAttenuation, const FVector4 IndirectLight[3])
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

void FHairWorksSceneProxy::DrawShadows(FRHICommandList& RHICmdList)
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

void FHairWorksSceneProxy::DrawShadow(const FViewMatrices& ViewMatrices, float DepthBias, float DepthScale)
{
	if (HairInstanceId == GFSDK_HairInstanceID_NULL)
		return;

	auto HairWorksSdk = GHairManager->GetHairworksSdk();

	// JDM: GROSS! - GET RID OF THIS. Need another member in the config that encapsulates this, since the shader already knows it's a shadow render.
	GFSDK_HairInstanceDescriptor HairDesc;
	HairWorksSdk->CopyCurrentInstanceDescriptor(HairInstanceId, HairDesc);

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

void FHairWorksSceneProxy::DrawBasePass(const FSceneView& View)
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

void FHairWorksSceneProxy::DrawVelocity(const FSceneView& View, const FViewMatrices& PrevViewMatrices)
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

void FHairWorksSceneProxy::CreateRenderThreadResources()
{
	FPrimitiveSceneProxy::CreateRenderThreadResources();

	auto HairWorksSdk = GHairManager->GetHairworksSdk();

	check(HairWorksSdk);

	UE_LOG(LogHairWorks, Log, TEXT("FHairSceneProxy::CreateRenderThreadResources"));

	check(AssetId != GFSDK_HairAssetID_NULL);

	HairWorksSdk->CreateHairInstance(AssetId, &HairInstanceId);

	UE_LOG(LogHairWorks, Log, TEXT("Created new hair instance %d"), static_cast<int32>(HairInstanceId));
}

FPrimitiveViewRelevance FHairWorksSceneProxy::GetViewRelevance(const FSceneView* View)
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

void FHairWorksSceneProxy::UpdateDynamicData_RenderThread(TSharedPtr<FDynamicRenderData> DynamicData)
{
	if (HairInstanceId == GFSDK_HairInstanceID_NULL)
		return;

	auto HairworksSdk = GHairManager->GetHairworksSdk();

	int numBones = DynamicData->BoneMatrices.Num();

	// Update bones
	HairworksSdk->UpdateSkinningMatrices(HairInstanceId, DynamicData->BoneMatrices.Num(), (gfsdk_float4x4*)DynamicData->BoneMatrices.GetData());

	// Update normal center bone
	auto HairDesc = DynamicData->HairInstanceDesc;

	if (DynamicData->NormalCenterBoneIndex != -1)
		HairDesc.m_hairNormalBoneIndex = DynamicData->NormalCenterBoneIndex;
	else
		HairDesc.m_hairNormalWeight = 0;

	// Merge global visualization flags.
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

#undef HairVisualizerCVarUpdate

	// Other
	HairDesc.m_modelToWorld = (gfsdk_float4x4&)GetLocalToWorld().M;
	HairDesc.m_useViewfrustrumCulling = false;

	// Set parameters to HairWorks
	HairworksSdk->UpdateInstanceDescriptor(HairInstanceId, HairDesc);	// Mainly for simulation.

	// Update textures
	check(DynamicData->Textures.Num() == GFSDK_HAIR_NUM_TEXTURES);
	HairTextures.SetNumZeroed(GFSDK_HAIR_NUM_TEXTURES);
	for (auto Idx = 0; Idx < HairTextures.Num(); ++Idx)
	{
		auto* Texture = DynamicData->Textures[Idx];
		if (Texture == nullptr)
			continue;

		if (Texture->Resource == nullptr)
			continue;

		HairTextures[Idx] = static_cast<FTexture2DResource*>(Texture->Resource)->GetTexture2DRHI();
	}

	for (auto Idx = 0; Idx < GFSDK_HAIR_NUM_TEXTURES; ++Idx)
	{
		auto TextureRef = HairTextures[Idx];
		auto Texture = static_cast<TD3D11Texture2D<FD3D11BaseTexture2D>*>(TextureRef.GetReference());
		HairworksSdk->SetTextureSRV(HairInstanceId, (GFSDK_HAIR_TEXTURE_TYPE)Idx, Texture ? Texture->GetShaderResourceView() : nullptr);
	}
}

