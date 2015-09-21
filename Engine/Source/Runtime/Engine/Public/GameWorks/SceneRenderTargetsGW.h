#pragma once
#include "RefCounting.h"

struct IPooledRenderTarget;

struct FSceneRenderTargetsGW
{
public:
	//TODO: ifdef this out for no HW
	//TODO: Make sure these init correctly.
// 	TRefCountPtr<IPooledRenderTarget> HairMask;
// 	TRefCountPtr<IPooledRenderTarget> HairLightAttenuation;
// 	TRefCountPtr<IPooledRenderTarget> HairDepthZ;

	FSceneRenderTargetsGW() { }

};