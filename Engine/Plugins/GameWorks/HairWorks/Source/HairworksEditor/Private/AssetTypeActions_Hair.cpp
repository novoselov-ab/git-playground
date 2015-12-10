#include "HairworksEditorPCH.h"
#include "HairWorksAsset.h"


UClass* FAssetTypeActions_Hair::GetSupportedClass() const
{
	return UHairWorksAsset::StaticClass();
}

void FAssetTypeActions_Hair::GetResolvedSourceFilePaths(const TArray<UObject*>& TypeAssets, TArray<FString>& OutSourceFilePaths) const
{
	for (auto Asset : TypeAssets)
	{
		auto Hair = CastChecked<UHairWorksAsset>(Asset);
		Hair->AssetImportData->ExtractFilenames(OutSourceFilePaths);
	}
}
