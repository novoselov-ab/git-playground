#include "HairworksEditorPCH.h"

#include "Hair.h"


UClass* FAssetTypeActions_Hair::GetSupportedClass() const
{
	return UHair::StaticClass();
}

void FAssetTypeActions_Hair::GetResolvedSourceFilePaths(const TArray<UObject*>& TypeAssets, TArray<FString>& OutSourceFilePaths) const
{
	for (auto& Asset : TypeAssets)
	{
		auto* Hair = CastChecked<UHair>(Asset);
		OutSourceFilePaths.Add(FReimportManager::ResolveImportFilename(Hair->SourceFilePath, Hair));
	}
}
