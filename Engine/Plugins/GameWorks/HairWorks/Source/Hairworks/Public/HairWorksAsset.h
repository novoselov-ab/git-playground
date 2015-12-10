#pragma once

#include "HairWorksAsset.generated.h"

class UHairWorksMaterial;

/*
	This class represents the hair asset.
*/
UCLASS()
class HAIRWORKS_API UHairWorksAsset : public UObject
{
	GENERATED_BODY()
public:

	UHairWorksAsset(const FObjectInitializer &ObjectInitializer);
	virtual ~UHairWorksAsset();

	UPROPERTY(EditAnywhere, Category = ImportSettings)
	bool bGroom = true;

	UPROPERTY(EditAnywhere, Category = ImportSettings)
	bool bMaterials = true;

	UPROPERTY(EditAnywhere, Category = ImportSettings)
	bool bConstraints = true;

	UPROPERTY(VisibleAnywhere, Category = ImportSettings)
	bool bTextures = false;

	UPROPERTY(EditAnywhere, Category = ImportSettings)
	bool bCollisions = true;

	UPROPERTY(VisibleAnywhere, Instanced, BlueprintReadOnly, Category = Hair)
	UHairWorksMaterial* HairMaterial;

	UPROPERTY()
	TArray<uint8> AssetData;

	UPROPERTY()
	TArray<FName> BoneNames;

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
