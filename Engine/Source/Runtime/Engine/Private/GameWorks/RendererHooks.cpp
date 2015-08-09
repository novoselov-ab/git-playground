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

void FRendererHooks::OnSetHairLight(FVector LightDirection, FLinearColor LightColor, bool bHairShadowed)
{
	for (auto callback : SetHairLightCallbacks.getCallbacks())
	{
		callback->getCallback()(LightDirection, LightColor, bHairShadowed);
	}
}

