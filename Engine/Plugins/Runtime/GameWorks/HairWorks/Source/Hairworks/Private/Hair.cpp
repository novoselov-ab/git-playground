#include "HairworksPCH.h"

const GFSDK_HairAssetID UHair::AssetIdNull = GFSDK_HairAssetID_NULL;

UHair::UHair(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
}

UHair::~UHair()
{
	// JDM: FIXME
	GHairManager->ReleaseHair_GameThread(AssetId);
}