#pragma once

#include "Hair.generated.h"

enum GFSDK_HairAssetID;

UCLASS()
class HAIRWORKS_API UHair : public UObject
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(EditAnywhere, Category = ImportSettings)
	FString SourceFilePath;
	
	UPROPERTY()
	TArray<uint8> AssetData;

	~UHair();

	GFSDK_HairAssetID AssetId = AssetIdNull;
	static const GFSDK_HairAssetID AssetIdNull;
};
