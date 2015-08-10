#include "HairworksPCH.h"
#include "Editor.h"

/*------------------------------------------------------------------------------
	UHairFactory.
------------------------------------------------------------------------------*/
UHairFactory::UHairFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UHair::StaticClass();
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

	auto Hair = NewObject<UHair>(InParent, Name, Flags);

//	auto* Hair = CastChecked<UHair>(StaticConstructObject(UHair::StaticClass(), InParent, Name, Flags));

	Hair->AssetData.SetNumUninitialized(BufferEnd - Buffer);
	FMemory::Memcpy(Hair->AssetData.GetData(), Buffer, Hair->AssetData.Num());

	return Hair;
}

bool UHairFactory::CanReimport(UObject* Obj, TArray<FString>& OutFilenames)
{
	auto* Hair = Cast<UHair>(Obj);
	if (!Hair)
		return false;

	if (!Hair->SourceFilePath.IsEmpty())
		OutFilenames.Add(Hair->SourceFilePath);

	return true;
}

void UHairFactory::SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths)
{
	auto* Hair = Cast<UHair>(Obj);
	if (Hair && ensure(NewReimportPaths.Num() == 1))
	{
		Hair->SourceFilePath = FReimportManager::SanitizeImportFilename(NewReimportPaths[0], Obj);
	}
}

EReimportResult::Type UHairFactory::Reimport(UObject* Obj)
{
	// Validate asset file.
	auto* Hair = Cast<UHair>(Obj);
	if (!Hair)
		return EReimportResult::Failed;

	TArray<uint8> FileData;
	FFileHelper::LoadFileToArray(FileData, *Hair->SourceFilePath);

	if (!GHairManager->IsHair_GameThread(FileData.GetData(), FileData.Num()))
		return EReimportResult::Failed;

	// Finish render thread work.
	FlushRenderingCommands();

	// Load asset
	Hair->AssetData = FileData;
	Hair->AssetId = UHair::AssetIdNull;

	// Notify components the change.
	for (TObjectIterator<UHairComponent> It; It; ++It)
	{
		if (It->Hair != Hair)
			continue;

		It->RecreateRenderState_Concurrent();
	}

	// Mark package dirty.
	(Obj->GetOuter() ? Obj->GetOuter() : Obj)->MarkPackageDirty();

	return EReimportResult::Succeeded;
}