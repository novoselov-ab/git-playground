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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Hair)
	FHairProperties	HairProperties;

	UPROPERTY()
	TArray<uint8> AssetData;

	// Bone names and indices.
	TMap<FName, int32> HairBoneToIdxMap;

	GFSDK_HairAssetID AssetId;

	// Load the hair asset on demand (when a component needs it)
	bool LoadHairAsset();

#if WITH_EDITORONLY_DATA
	// Import data for this 
	UPROPERTY(VisibleAnywhere, Instanced, Category = ImportSettings)
	class UAssetImportData* AssetImportData;

	// UObject interface
	virtual void PostInitProperties() override;
	virtual void Serialize(FArchive& Ar) override;
	// End of UObject interface
#endif

};
