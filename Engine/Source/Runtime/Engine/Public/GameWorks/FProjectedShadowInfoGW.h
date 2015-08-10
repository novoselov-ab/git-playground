#pragma once

struct FProjectedShadowInfoGW
{
	mutable uint32 bHairReceiver : 1;
	mutable uint32 bHairRenderProjection : 1;

	FProjectedShadowInfoGW() : bHairReceiver(0), bHairRenderProjection(0) { }
};