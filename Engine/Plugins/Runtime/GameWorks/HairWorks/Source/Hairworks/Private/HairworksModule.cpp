#include "HairworksPCH.h"

FHairManager* GHairManager = nullptr;

IMPLEMENT_MODULE( FHairworksModule, HairWorks );
DEFINE_LOG_CATEGORY(LogHairWorks);

void FHairworksModule::StartupModule()
{
	// Register asset types
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	HairAssetTypeActions = MakeShareable(new FAssetTypeActions_Hair);
	AssetTools.RegisterAssetTypeActions(HairAssetTypeActions.ToSharedRef());

	// Get it, which causes it to init.
	HairManager = MakeUnique<FHairManager>();
	
	// Not happy about exposing this raw pointer
	GHairManager = HairManager.Get();
}

void FHairworksModule::ShutdownModule()
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
