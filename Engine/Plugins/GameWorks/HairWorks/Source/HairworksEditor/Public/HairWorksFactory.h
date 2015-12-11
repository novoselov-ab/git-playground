#pragma once

#include "Factories/Factory.h"
#include "EditorReimportHandler.h"
#include "HairWorksFactory.generated.h"

class UHairWorksAsset;

UCLASS()
class UHairWorksFactory : public UFactory, public FReimportHandler
{
	GENERATED_UCLASS_BODY()

	bool FactoryCanImport(const FString& Filename)override;
	//	FText GetDisplayName() const override;
	UObject* FactoryCreateBinary(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, const TCHAR* Type, const uint8*& Buffer, const uint8* BufferEnd, FFeedbackContext* Warn) override;
	bool CanReimport(UObject* Obj, TArray<FString>& OutFilenames) override;
	void SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths) override;
	EReimportResult::Type Reimport(UObject* Obj) override;
private:
	FString FixupBoneName(const FString &InBoneName);
	void InitHairAssetInfo(UHairWorksAsset& Hair, GFSDK_HairAssetID HairAssetId, const GFSDK_HairInstanceDescriptor* NewInstanceDesc = nullptr);
};


