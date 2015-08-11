#include "HairworksEditorPCH.h"

IMPLEMENT_MODULE( FHairworksEditorModule, HairWorksEditor );
//DEFINE_LOG_CATEGORY(LogHairWorksEditor);

void FHairworksEditorModule::StartupModule()
{
	// Register asset types
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	HairAssetTypeActions = MakeShareable(new FAssetTypeActions_Hair);
	AssetTools.RegisterAssetTypeActions(HairAssetTypeActions.ToSharedRef());

}

void FHairworksEditorModule::ShutdownModule()
{
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
		if (HairAssetTypeActions.IsValid())
		{
			AssetTools.UnregisterAssetTypeActions(HairAssetTypeActions.ToSharedRef());
		}
	}
}
