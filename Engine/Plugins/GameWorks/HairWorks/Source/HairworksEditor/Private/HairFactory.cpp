#include "HairworksEditorPCH.h"
#include "Editor.h"
#include "HairworksModule.h"
#include "HairManager.h"
#include "HairWorksComponent.h"

/*------------------------------------------------------------------------------
	UHairFactory.
------------------------------------------------------------------------------*/
UHairFactory::UHairFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UHairWorksAsset::StaticClass();
	bEditorImport = true;
	bCreateNew = false;
	Formats.Add(TEXT("apx;HairWorks Asset"));
	Formats.Add(TEXT("apb;HairWorks Asset"));
}

bool UHairFactory::FactoryCanImport(const FString& Filename)
{
	// Skip APEX file that is not hair.
	TArray<uint8> Buffer;
	FFileHelper::LoadFileToArray(Buffer, *Filename);
	return GHairManager->IsHair_GameThread(Buffer.GetData(), Buffer.Num());
}

// FText UHairFactory::GetDisplayName() const
// {
// 	return LOCTEXT("HairFactory", "Hair");
// }

UObject* UHairFactory::FactoryCreateBinary(
	UClass*				Class,
	UObject*			InParent,
	FName				Name,
	EObjectFlags		Flags,
	UObject*			Context,
	const TCHAR*		FileType,
	const uint8*&		Buffer,
	const uint8*			BufferEnd,
	FFeedbackContext*	Warn
	)
{
	// Notify
	FEditorDelegates::OnAssetPreImport.Broadcast(this, Class, InParent, Name, FileType);

	// Check asset
	if (!GHairManager->IsHair_GameThread(Buffer, BufferEnd - Buffer))
		return nullptr;

	// Create asset

	auto Hair = NewObject<UHairWorksAsset>(InParent, Name, Flags);

	Hair->AssetData.SetNumUninitialized(BufferEnd - Buffer);
	FMemory::Memcpy(Hair->AssetData.GetData(), Buffer, Hair->AssetData.Num());

	return Hair;
}

bool UHairFactory::CanReimport(UObject* Obj, TArray<FString>& OutFilenames)
{
	auto Hair = Cast<UHairWorksAsset>(Obj);
	if (Hair && Hair->AssetImportData)
	{
		Hair->AssetImportData->ExtractFilenames(OutFilenames);
		return true;
	}
	return false;
}

void UHairFactory::SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths)
{
	auto Hair = Cast<UHairWorksAsset>(Obj);
	if (Hair && ensure(NewReimportPaths.Num() == 1))
	{
		Hair->AssetImportData->UpdateFilenameOnly(NewReimportPaths[0]);
	}
}

EReimportResult::Type UHairFactory::Reimport(UObject* Obj)
{
	// Validate asset file.
	auto Hair = Cast<UHairWorksAsset>(Obj);
	if (!Hair)
	{
		UE_LOG(LogHairWorksEditor, Error, TEXT("Failed to import hair asset - Hair is null."));
		return EReimportResult::Failed;
	}

	// Make sure file is valid and exists
	const FString Filename = Hair->AssetImportData->GetFirstFilename();
	if (!Filename.Len() || IFileManager::Get().FileSize(*Filename) == INDEX_NONE)
	{
		return EReimportResult::Failed;
	}

	TArray<uint8> FileData;
	if (!FFileHelper::LoadFileToArray(FileData, *Filename))
	{
		UE_LOG(LogHairWorksEditor, Error, TEXT("Failed to import hair asset from file %s - IO error"), *Filename);
		return EReimportResult::Failed;
	}

	if (!GHairManager->IsHair_GameThread(FileData.GetData(), FileData.Num()))
	{
		UE_LOG(LogHairWorksEditor, Error, TEXT("Failed to import hair asset from file %s - Not a hair asset."), *Filename);
		return EReimportResult::Failed;
	}

	// Load asset
	Hair->AssetData = FileData;

//	GHairManager->GetHairInfo(Hair->HairBoneToIdxMap, Hair);

	// Notify components the change.
// 	for (TObjectIterator<UHairWorksComponent> It; It; ++It)
// 	{
// 		if (It->Hair == Hair)
// 		{
// 			It->HairProperties = Hair->HairProperties;
// 			It->RecreateRenderState_Concurrent();
// 		}
// 	}

	// Mark package dirty.
	(Obj->GetOuter() ? Obj->GetOuter() : Obj)->MarkPackageDirty();

	return EReimportResult::Succeeded;
}