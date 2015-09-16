#pragma once

#include "HairProperties.h"
#include "Hair.generated.h"


/*
	This class represents the hair asset.
*/
UCLASS()
class HAIRWORKS_API UHair : public UObject
{
	GENERATED_BODY()
public:

	UHair(const FObjectInitializer &ObjectInitializer);
	virtual ~UHair();

	UPROPERTY()
	UAssetImportData* AssetImportData;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Hair)
	FHairProperties	HairProperties;

	UPROPERTY()
	TArray<uint8> AssetData;

	// Bone names and indices.
	TMap<FName, int32> HairBoneToIdxMap;

	GFSDK_HairAssetID AssetId;

	// Load the hair asset on demand (when a component needs it)
	bool LoadHairAsset();
};
