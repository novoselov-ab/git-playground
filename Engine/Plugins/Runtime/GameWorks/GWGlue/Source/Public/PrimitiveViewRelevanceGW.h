#pragma once

struct FPrimitiveViewRelevanceGW
{
public:
//TODO: ifdef out if no HW
	uint32 bHair : 1;

	FPrimitiveViewRelevanceGW() : bHair(0) { }
};