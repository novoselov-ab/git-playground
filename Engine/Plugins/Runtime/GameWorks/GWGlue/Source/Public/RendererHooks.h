#pragma once
#include <functional>
#include "SortedCallbackList.h"

class FViewInfo;
class FRHICommandList;
struct FVector;
struct FLinearColor;

typedef TArray<const FPrimitiveSceneInfo*, SceneRenderingAllocator> PrimitiveArrayType;

/*
	This class is called by UE4s rendering layer in multiple places, to allow the GW plugins to do their rendering where they need to without impacting UE4 with a lot of pasted in code.

	When UE4 calls one of the hooks, that may branch out to multiple GW plugin render calls, all in the order they think they should be in. 

	NB: This class is a singleton. Since it needs to maintain state, I prefer this to static. Also, injecting or passing this around would be a real pain.
*/
class GWGLUE_API FRendererHooks
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

	void OnRenderBasePassDynamic(const FViewInfo &View, FRHICommandList &RHICmdList);
	
	// TODO: Make this interface more consistent.
	void OnRenderDepthDynamic(const FViewInfo *View, PrimitiveArrayType SubjectPrimitives, FViewMatrices ViewMatrices, float ShaderDepthBias, float InvMaxSubjectDepth);

	void OnSetHairLight(FVector LightDirection, FLinearColor LightColor, bool bHairShadowed);

	// Subscription methods

	// Callback lists
	FSortedCallbackList<std::function<void(const FViewInfo&)>> TranslucentViewRenderCallbacks;
	FSortedCallbackList<std::function<void(const FViewInfo&)>> RenderVelocitiesInnerCallbacks;
	
	FSortedCallbackList<std::function<void(const FViewInfo&, FRHICommandList&)>> RenderBasePassDynamicCallbacks;

	FSortedCallbackList<std::function<void(const FViewInfo*, PrimitiveArrayType, FViewMatrices, float, float)>> RenderDepthDynamicCallbacks;

	FSortedCallbackList<std::function<void(FVector, FLinearColor, bool)>> SetHairLightCallbacks;

	// Unsub methods? Do we ever want to do that? Probably not.

private:
	FRendererHooks(const FRendererHooks &) = delete;
	FRendererHooks& operator=(const FRendererHooks &) = delete;

};