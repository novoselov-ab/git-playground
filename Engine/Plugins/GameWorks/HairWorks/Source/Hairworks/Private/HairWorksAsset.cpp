#include "HairworksPCH.h"

UHairWorksAsset::UHairWorksAsset(const class FObjectInitializer& ObjectInitializer): 
Super(ObjectInitializer),
HairMaterial(nullptr),
AssetData(),
HairBoneToIdxMap(),
AssetId(GFSDK_HairAssetID_NULL)
{
}

UHairWorksAsset::~UHairWorksAsset()
{
	if (AssetId != GFSDK_HairAssetID_NULL)
	{
		GHairManager->ReleaseHair_GameThread(AssetId);
	}
}

bool UHairWorksAsset::LoadHairAsset()
{
	if (AssetId != GFSDK_HairAssetID_NULL)
	{
		UE_LOG(LogHairWorks, Warning, TEXT("Tried to load a hair asset but it was already loaded."));
		return true;
	}

	//JDM: Load has to be in a render thread, so enqueue it.

	ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
		LoadAsset,
		TArray<uint8>, AssetData, AssetData,
		GFSDK_HairAssetID&, AssetId, AssetId,
		{
			GFSDK_HairConversionSettings LoadSettings;
			LoadSettings.m_targetHandednessHint = GFSDK_HAIR_HANDEDNESS_HINT::GFSDK_HAIR_LEFT_HANDED;
			LoadSettings.m_targetUpAxisHint = GFSDK_HAIR_UP_AXIS_HINT::GFSDK_HAIR_Z_UP;
			auto result = GHairManager->GetHairworksSdk()->LoadHairAssetFromMemory(AssetData.GetData(), AssetData.Num(), &AssetId, 0, &LoadSettings);

			if (result != GFSDK_HAIR_RETURN_OK)
			{
				UE_LOG(LogHairWorks, Error, TEXT("Failed to load hair asset from memory."));
				AssetId = GFSDK_HairAssetID_NULL;
				return;
			}
		}
	);

	// Wait
	FRenderCommandFence RenderCmdFenc;
	RenderCmdFenc.BeginFence();
	RenderCmdFenc.Wait();

	gfsdk_U32 BoneNum = 0;
	GHairManager->GetHairworksSdk()->GetNumBones(AssetId, &BoneNum);

	UE_LOG(LogHairWorks, Log, TEXT("Loaded asset: Bone count %d, asset %d"), BoneNum, static_cast<int32>(AssetId));

	return true;
}

#if WITH_EDITORONLY_DATA

void UHairWorksAsset::PostInitProperties()
{
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		AssetImportData = NewObject<UAssetImportData>(this, TEXT("AssetImportData"));
	}
	Super::PostInitProperties();
}

void UHairWorksAsset::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	if (Ar.IsLoading() && Ar.UE4Ver() < VER_UE4_ASSET_IMPORT_DATA_AS_JSON && !AssetImportData)
	{
		// AssetImportData should always be valid
		AssetImportData = NewObject<UAssetImportData>(this, TEXT("AssetImportData"));
	}
}

#endif
