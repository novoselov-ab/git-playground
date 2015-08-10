#pragma once
#include <functional>
#include "SortedCallbackList.h"

class FViewInfo;
class FRHICommandList;
struct FVector;
struct FLinearColor;
class FProjectedShadowInfo;

typedef TArray<const FPrimitiveSceneInfo*, SceneRenderingAllocator> PrimitiveArrayType;

/*
	This class is called by UE4s rendering layer in multiple places, to allow the GW plugins to do their rendering where they need to without impacting UE4 with a lot of pasted in code.

	When UE4 calls one of the hooks, that may branch out to multiple GW plugin render calls, all in the order they think they should be in. 

	NB: This class is a singleton. Since it needs to maintain state, I prefer this to static. Also, injecting or passing this around would be a real pain.
*/
class ENGINE_API FRendererHooks
{
public:
	static FRendererHooks& get()
	{
		static FRendererHooks instance;

		return instance;
	}

	FRendererHooks() = default;


	// TODO: Better, more generic names!
	void OnTranslucentViewRender(const FViewInfo &View);
	void OnRenderVelocitiesInnner(const FViewInfo &View);

	void OnRenderBasePassView(const FViewInfo &View);
	void OnRenderBasePassDynamic(const FViewInfo &View, FRHICommandList &RHICmdList);
	
	// TODO: Make this interface more consistent.
	void OnProjectedShadowRenderDepthDynamic(const FViewInfo *View, PrimitiveArrayType SubjectPrimitives, FViewMatrices ViewMatrices, float ShaderDepthBias, float InvMaxSubjectDepth);

	void OnProjectedShadowPreShadow(const FProjectedShadowInfo& ShadowInfo, const FViewInfo &View, PrimitiveArrayType &ReceiverPrimitives);
	void OnProjectedShadowRenderProjection(const FProjectedShadowInfo& shadowInfo, const FViewInfo& View, FRHICommandList& RHICmdList);
	void OnProjectedShadowRenderProjectionEnd(const FProjectedShadowInfo& ShadowInfo, const FViewInfo &View, int32 ViewIndex, FRHICommandList& RHICmdList);

	// JDM: This is really hacky - figure out a way to remove this.
	void OnSetHairLight(FVector LightDirection, FLinearColor LightColor, bool bHairShadowed);

	void OnPostVisibilityFrameSetup(FViewInfo &View);

	// Subscription methods

	// Callback lists
	FSortedCallbackList<std::function<void(const FViewInfo&)>> TranslucentViewRenderCallbacks;
	FSortedCallbackList<std::function<void(const FViewInfo&)>> RenderVelocitiesInnerCallbacks;

	FSortedCallbackList<std::function<void(const FViewInfo&)>> RenderBasePassViewCallbacks;
	
	FSortedCallbackList<std::function<void(const FViewInfo&, FRHICommandList&)>> RenderBasePassDynamicCallbacks;

	FSortedCallbackList<std::function<void(const FViewInfo*, PrimitiveArrayType, FViewMatrices, float, float)>> RenderProjectedShadowDepthDynamicCallbacks;

	FSortedCallbackList<std::function<void(const FProjectedShadowInfo&, const FViewInfo&, PrimitiveArrayType &) >> RenderProjectedShadowPreShadowCallbacks;
	FSortedCallbackList<std::function<void(const FProjectedShadowInfo&, const FViewInfo&, FRHICommandList&)>> RenderProjectedShadowRenderProjectionCallbacks;

	FSortedCallbackList<std::function<void(const FProjectedShadowInfo&, const FViewInfo&, int32 ViewIndex, FRHICommandList&)>> RenderProjectedShadowRenderProjectionEndCallbacks;

	FSortedCallbackList<std::function<void(FVector, FLinearColor, bool)>> SetHairLightCallbacks;

	FSortedCallbackList<std::function<void(FViewInfo&)>> PostVisibilityFrameSetupCallbacks;

	// Unsub methods? Do we ever want to do that? Probably not.

private:
	FRendererHooks(const FRendererHooks &) = delete;
	FRendererHooks& operator=(const FRendererHooks &) = delete;
};