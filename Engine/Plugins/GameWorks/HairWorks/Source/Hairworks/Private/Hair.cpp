#include "HairworksPCH.h"

UHair::UHair(const class FObjectInitializer& ObjectInitializer): 
Super(ObjectInitializer),
AssetImportData(nullptr),
HairProperties(),
AssetData(),
HairBoneToIdxMap(),
AssetId(GFSDK_HairAssetID_NULL)
{
}

UHair::~UHair()
{
	if (AssetId != GFSDK_HairAssetID_NULL)
	{
		GHairManager->ReleaseHair_GameThread(AssetId);
	}
}

bool UHair::LoadHairAsset()
{
	if (AssetId != GFSDK_HairAssetID_NULL)
	{
		UE_LOG(LogHairWorks, Warning, TEXT("Tried to load a hair asset but it was already loaded."));
		return true;
	}

	GFSDK_HairConversionSettings LoadSettings;
	LoadSettings.m_targetHandednessHint = GFSDK_HAIR_HANDEDNESS_HINT::GFSDK_HAIR_LEFT_HANDED;
	LoadSettings.m_targetUpAxisHint = GFSDK_HAIR_UP_AXIS_HINT::GFSDK_HAIR_Z_UP;
	auto result = GHairManager->GetHairworksSdk()->LoadHairAssetFromMemory(AssetData.GetData(), AssetData.Num(), &AssetId, 0, &LoadSettings);

	if (result != GFSDK_HAIR_RETURN_OK)
	{
		UE_LOG(LogHairWorks, Error, TEXT("Failed to load hair asset from memory."));
		AssetId = GFSDK_HairAssetID_NULL;
		return false;
	}

	return true;
}
