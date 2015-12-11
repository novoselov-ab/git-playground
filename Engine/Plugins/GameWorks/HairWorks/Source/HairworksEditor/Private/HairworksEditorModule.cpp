#include "HairworksEditorPCH.h"
#include "PropertyEditorModule.h"

IMPLEMENT_MODULE( FHairworksEditorModule, HairWorksEditor );
DEFINE_LOG_CATEGORY(LogHairWorksEditor);

void FHairworksEditorModule::StartupModule()
{
	// Register asset types
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	HairAssetTypeActions = MakeShareable(new FAssetTypeActions_Hair);
	AssetTools.RegisterAssetTypeActions(HairAssetTypeActions.ToSharedRef());

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	PropertyModule.RegisterCustomClassLayout("HairWorksMaterial", FOnGetDetailCustomizationInstance::CreateStatic(&FHairWorksMaterialDetails::MakeInstance));

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
