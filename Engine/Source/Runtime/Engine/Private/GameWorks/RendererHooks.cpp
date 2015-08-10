#include "EnginePrivate.h"

void FRendererHooks::OnTranslucentViewRender(const FViewInfo &View)
{
	for (auto callback : TranslucentViewRenderCallbacks.getCallbacks())
	{
		callback->getCallback()(View);
	}
}

void FRendererHooks::OnRenderVelocitiesInnner(const FViewInfo &View)
{
	for (auto callback : RenderVelocitiesInnerCallbacks.getCallbacks())
	{
		callback->getCallback()(View);
	}
}

void FRendererHooks::OnRenderBasePassView(const FViewInfo &View)
{
	for (auto callback : RenderBasePassViewCallbacks.getCallbacks())
	{
		callback->getCallback()(View);
	}
}

void FRendererHooks::OnRenderBasePassDynamic(const FViewInfo &View, FRHICommandList &RHICmdList)
{
	for (auto callback : RenderBasePassDynamicCallbacks.getCallbacks())
	{
		callback->getCallback()(View, RHICmdList);
	}
}

void FRendererHooks::OnProjectedShadowRenderDepthDynamic(const FViewInfo *View, PrimitiveArrayType SubjectPrimitives, FViewMatrices ViewMatrices, float ShaderDepthBias, float InvMaxSubjectDepth)
{
	for (auto callback : RenderProjectedShadowDepthDynamicCallbacks.getCallbacks())
	{
		callback->getCallback()(View, SubjectPrimitives, ViewMatrices, ShaderDepthBias, InvMaxSubjectDepth);
	}
}

void FRendererHooks::OnProjectedShadowRenderProjection(const FProjectedShadowInfo& ShadowInfo, const FViewInfo &View, FRHICommandList &RHICmdList)
{
	for (auto callback : RenderProjectedShadowRenderProjectionCallbacks.getCallbacks())
	{
		callback->getCallback()(ShadowInfo, View, RHICmdList);
	}
}

void FRendererHooks::OnSetHairLight(FVector LightDirection, FLinearColor LightColor, bool bHairShadowed)
{
	for (auto callback : SetHairLightCallbacks.getCallbacks())
	{
		callback->getCallback()(LightDirection, LightColor, bHairShadowed);
	}
}

void FRendererHooks::OnPostVisibilityFrameSetup(FViewInfo &View)
{
	for (auto callback : PostVisibilityFrameSetupCallbacks.getCallbacks())
	{
		callback->getCallback()(View);
	}
}

void FRendererHooks::OnProjectedShadowPreShadow(const FProjectedShadowInfo& ShadowInfo, const FViewInfo &View, PrimitiveArrayType &ReceiverPrimitives)
{
	for (auto callback : RenderProjectedShadowPreShadowCallbacks.getCallbacks())
	{
		callback->getCallback()(ShadowInfo, View, ReceiverPrimitives);
	}
}

