#include "EnginePrivate.h"

void FRendererHooks::OnTranslucentViewRender(const FViewInfo &View, FRHICommandList &RHICmdList)
{
	for (auto callback : TranslucentViewRenderCallbacks.getCallbacks())
	{
		callback->getCallback()(View, RHICmdList);
	}
}

void FRendererHooks::OnRenderVelocitiesInnner(const FViewInfo &View)
{
	for (auto callback : RenderVelocitiesInnerCallbacks.getCallbacks())
	{
		callback->getCallback()(View);
	}
}

void FRendererHooks::OnRenderBasePassView(FViewInfo &View)
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

void FRendererHooks::OnProjectedShadowRenderDepthDynamic(const FViewInfo *View, TArray<const FPrimitiveSceneInfo*, SceneRenderingAllocator> SubjectPrimitives, FViewMatrices ViewMatrices, float ShaderDepthBias, float InvMaxSubjectDepth)
{
	for (auto callback : RenderProjectedShadowDepthDynamicCallbacks.getCallbacks())
	{
		callback->getCallback()(View, SubjectPrimitives, ViewMatrices, ShaderDepthBias, InvMaxSubjectDepth);
	}
}

void FRendererHooks::OnProjectedShadowPreShadow(const FProjectedShadowInfo& ShadowInfo, const FViewInfo &View, const TArray<const FPrimitiveSceneInfo*, SceneRenderingAllocator> &ReceiverPrimitives)
{
	for (auto callback : RenderProjectedShadowPreShadowCallbacks.getCallbacks())
	{
		callback->getCallback()(ShadowInfo, View, ReceiverPrimitives);
	}
}


void FRendererHooks::OnProjectedShadowRenderProjection(const FProjectedShadowInfo& ShadowInfo, const FViewInfo &View, FRHICommandList &RHICmdList)
{
	for (auto callback : RenderProjectedShadowRenderProjectionCallbacks.getCallbacks())
	{
		callback->getCallback()(ShadowInfo, View, RHICmdList);
	}
}

void FRendererHooks::OnProjectedShadowRenderProjectionEnd(const FProjectedShadowInfo& ShadowInfo, const FViewInfo &View, int32 ViewIndex, FRHICommandListImmediate& RHICmdList)
{
	for (auto callback : RenderProjectedShadowRenderProjectionEndCallbacks.getCallbacks())
	{
		callback->getCallback()(ShadowInfo, View, ViewIndex, RHICmdList);
	}
}

void FRendererHooks::OnAfterRenderProjection(const FProjectedShadowInfo& ShadowInfo, const FViewInfo &View, int32 ViewIndex, FRHICommandListImmediate& RHICmdList)
{
	for (auto callback : AfterRenderProjectionCallbacks.getCallbacks())
	{
		callback->getCallback()(ShadowInfo, View, ViewIndex, RHICmdList);
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

void FRendererHooks::OnAllocLightAttenuation(FPooledRenderTargetDesc Desc)
{
	for (auto callback : AllocLightAttenuationCallbacks.getCallbacks())
	{
		callback->getCallback()(Desc);
	}

}

void FRendererHooks::OnAllocCommonDepthTargets(FPooledRenderTargetDesc Desc)
{
	for (auto callback : AllocCommonDepthTargetsCallbacks.getCallbacks())
	{
		callback->getCallback()(Desc);
	}
}

void FRendererHooks::OnAllocSceneColor(FPooledRenderTargetDesc Desc)
{
	for (auto callback : AllocSceneColorCallbacks.getCallbacks())
	{
		callback->getCallback()(Desc);
	}
}

void FRendererHooks::OnDeallocRenderTargets()
{
	for (auto callback : DeallocRenderTargetsCallbacks.getCallbacks())
	{
		callback->getCallback()();
	}
}
