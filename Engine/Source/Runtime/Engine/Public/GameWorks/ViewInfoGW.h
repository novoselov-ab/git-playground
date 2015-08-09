#pragma once

struct FViewInfoGW
{
public:

//TODO: ifdef out by plugin
	uint32 bHasHair : 1;

	FViewInfoGW() : bHasHair(0) { }
};