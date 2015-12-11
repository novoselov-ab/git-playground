#include "HairworksEditorPCH.h"
#include "Editor.h"
#include "HairWorksMaterial.h"
#include "HairworksModule.h"
#include "HairManager.h"
#include "HairWorksComponent.h"
#include "SkelImport.h"
#include "HairWorksAsset.h"

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

// UObject* UHairFactory::FactoryCreateBinary(
// 	UClass*				Class,
// 	UObject*			InParent,
// 	FName				Name,
// 	EObjectFlags		Flags,
// 	UObject*			Context,
// 	const TCHAR*		FileType,
// 	const uint8*&		Buffer,
// 	const uint8*			BufferEnd,
// 	FFeedbackContext*	Warn
// 	)
// {
// 	// Notify
// 	FEditorDelegates::OnAssetPreImport.Broadcast(this, Class, InParent, Name, FileType);
// 
// 	// Check asset
// 	if (!GHairManager->IsHair_GameThread(Buffer, BufferEnd - Buffer))
// 		return nullptr;
// 
// 	// Create asset
// 
// 	auto Hair = NewObject<UHairWorksAsset>(InParent, Name, Flags);
// 
// 	Hair->AssetData.SetNumUninitialized(BufferEnd - Buffer);
// 	FMemory::Memcpy(Hair->AssetData.GetData(), Buffer, Hair->AssetData.Num());
// 
// 	return Hair;
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
	FEditorDelegates::OnAssetPreImport.Broadcast(this, Class, InParent, Name, FileType);

	auto HairWorksSDK = GHairManager->GetHairworksSdk();

	// Create real hair asset to get basic asset information
	auto HairAssetId = GFSDK_HairAssetID_NULL;

	HairWorksSDK->LoadHairAssetFromMemory(Buffer, BufferEnd - Buffer, &HairAssetId, nullptr, &GHairManager->HairWorksConversionSettings);
	if (HairAssetId == GFSDK_HairAssetID_NULL)
	{
		FEditorDelegates::OnAssetPostImport.Broadcast(this, nullptr);
		return nullptr;
	}

	// Create UHairWorksAsset
	auto* Hair = NewObject<UHairWorksAsset>(InParent, Name, Flags);

	// Initialize hair
	InitHairAssetInfo(*Hair, HairAssetId);

	// Clear temporary hair asset
	HairWorksSDK->FreeHairAsset(HairAssetId);

	// Set path
//	Hair->SourceFilePath = FReimportManager::SanitizeImportFilename(*CurrentFilename, Hair);

	// Set data
	Hair->AssetData.Append(Buffer, BufferEnd - Buffer);

	FEditorDelegates::OnAssetPostImport.Broadcast(this, Hair);

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

// EReimportResult::Type UHairFactory::Reimport(UObject* Obj)
// {
// 	// Validate asset file.
// 	auto Hair = Cast<UHairWorksAsset>(Obj);
// 	if (!Hair)
// 	{
// 		UE_LOG(LogHairWorksEditor, Error, TEXT("Failed to import hair asset - Hair is null."));
// 		return EReimportResult::Failed;
// 	}
// 
// 	// Make sure file is valid and exists
// 	const FString Filename = Hair->AssetImportData->GetFirstFilename();
// 	if (!Filename.Len() || IFileManager::Get().FileSize(*Filename) == INDEX_NONE)
// 	{
// 		return EReimportResult::Failed;
// 	}
// 
// 	TArray<uint8> FileData;
// 	if (!FFileHelper::LoadFileToArray(FileData, *Filename))
// 	{
// 		UE_LOG(LogHairWorksEditor, Error, TEXT("Failed to import hair asset from file %s - IO error"), *Filename);
// 		return EReimportResult::Failed;
// 	}
// 
// 	if (!GHairManager->IsHair_GameThread(FileData.GetData(), FileData.Num()))
// 	{
// 		UE_LOG(LogHairWorksEditor, Error, TEXT("Failed to import hair asset from file %s - Not a hair asset."), *Filename);
// 		return EReimportResult::Failed;
// 	}
// 
// 	// Load asset
// 	Hair->AssetData = FileData;
// 
// //	GHairManager->GetHairInfo(Hair->HairBoneToIdxMap, Hair);
// 
// 	// Notify components the change.
// // 	for (TObjectIterator<UHairWorksComponent> It; It; ++It)
// // 	{
// // 		if (It->Hair == Hair)
// // 		{
// // 			It->HairProperties = Hair->HairProperties;
// // 			It->RecreateRenderState_Concurrent();
// // 		}
// // 	}
// 
// 	// Mark package dirty.
// 	(Obj->GetOuter() ? Obj->GetOuter() : Obj)->MarkPackageDirty();
// 
// 	return EReimportResult::Succeeded;
// }

EReimportResult::Type UHairFactory::Reimport(UObject* Obj)
{
	// Finish render thread work.
	FlushRenderingCommands();

	auto HairWorksSDK = GHairManager->GetHairworksSdk();

	// Validate asset
	auto* Hair = Cast<UHairWorksAsset>(Obj);
	if (Hair == nullptr)
		return EReimportResult::Failed;

	auto NewHairAssetId = GFSDK_HairAssetID_NULL;
	{
		// Make sure file is valid and exists
		const FString Filename = Hair->AssetImportData->GetFirstFilename();
		if (!Filename.Len() || IFileManager::Get().FileSize(*Filename) == INDEX_NONE)
		{
			return EReimportResult::Failed;
		}

		// Load file
		TArray<uint8> FileData;
		if (!FFileHelper::LoadFileToArray(FileData, *Filename))
		{
			UE_LOG(LogHairWorksEditor, Error, TEXT("Can't load file [%s]"), *Filename);
			return EReimportResult::Failed;
		}

		// Create HairWorks asset
		HairWorksSDK->LoadHairAssetFromMemory(FileData.GetData(), FileData.Num(), &NewHairAssetId, nullptr, &GHairManager->HairWorksConversionSettings);
		if (NewHairAssetId == GFSDK_HairAssetID_NULL)
		{
			UE_LOG(LogHairWorksEditor, Error, TEXT("Can't create Hair asset"));
			return EReimportResult::Failed;
		}
	}

	// Create target hair that we will copy things to
	auto TgtHairAssetId = Hair->AssetId;
	Hair->AssetId = GFSDK_HairAssetID_NULL;	// HairWorks asset must be recreated.
	if (TgtHairAssetId == GFSDK_HairAssetID_NULL)
	{
		HairWorksSDK->LoadHairAssetFromMemory(Hair->AssetData.GetData(), Hair->AssetData.Num(), &TgtHairAssetId, nullptr, &GHairManager->HairWorksConversionSettings);
	}

	check(TgtHairAssetId != GFSDK_HairAssetID_NULL);

	// Copy asset content
	GFSDK_HairInstanceDescriptor NewInstanceDesc;
	{
		HairWorksSDK->CopyInstanceDescriptorFromAsset(NewHairAssetId, NewInstanceDesc);

		GFSDK_HairAssetCopySettings CopySettings;
		CopySettings.m_copyAll = false;
		CopySettings.m_copyCollision = Hair->bCollisions;
		CopySettings.m_copyConstraints = Hair->bConstraints;
		CopySettings.m_copyGroom = Hair->bGroom;
		CopySettings.m_copyTextures = Hair->bTextures;
		HairWorksSDK->CopyAsset(NewHairAssetId, TgtHairAssetId, CopySettings);

		// Finished copy. Clear.
		HairWorksSDK->FreeHairAsset(NewHairAssetId);
		NewHairAssetId = GFSDK_HairAssetID_NULL;
	}

	// Initialize hair
	InitHairAssetInfo(*Hair, TgtHairAssetId, &NewInstanceDesc);

	// Stream the updated HairWorks asset to asset data.
	{
		void* HairAssetData = nullptr;
		uint32 HairAssetDataSize = 0;

		gfsdk_new_delete_t CustomAllocator;
		CustomAllocator.new_ = [](size_t Size)->void*
		{return FMemory::Malloc(Size); };
		CustomAllocator.delete_ = [](void* Memory)
		{FMemory::Free(Memory); };

		HairWorksSDK->SaveHairAssetToMemory(HairAssetData, HairAssetDataSize, &CustomAllocator, true, TgtHairAssetId);

		// Set data to hair
		Hair->AssetData.Empty(HairAssetDataSize);
		Hair->AssetData.Append(static_cast<uint8*>(HairAssetData), HairAssetDataSize);

		// Clear memory
		FMemory::Free(HairAssetData);
		HairAssetData = nullptr;

		// Finished streaming. Clear.
		HairWorksSDK->FreeHairAsset(TgtHairAssetId);
		TgtHairAssetId = GFSDK_HairAssetID_NULL;
	}

	// Notify components the change.
	for (TObjectIterator<UHairWorksComponent> It; It; ++It)
	{
		if (It->HairInstance.Hair != Hair)
			continue;

		It->RecreateRenderState_Concurrent();
	}

	// Mark package dirty.
	(Obj->GetOuter() ? Obj->GetOuter() : Obj)->MarkPackageDirty();

	return EReimportResult::Succeeded;
}

// C&P of the one in FSkeletalMeshImportData as it's not exported.
FString UHairFactory::FixupBoneName(const FString &InBoneName)
{
	FString BoneName = InBoneName;

	BoneName.Trim();
	BoneName.TrimTrailing();
	BoneName = BoneName.Replace(TEXT(" "), TEXT("-"));

	return BoneName;
}

void UHairFactory::InitHairAssetInfo(UHairWorksAsset& Hair, GFSDK_HairAssetID HairAssetId, const GFSDK_HairInstanceDescriptor* NewInstanceDesc)
{
	auto HairWorksSDK = GHairManager->GetHairworksSdk();

	// Get bones. Used for bone remapping, etc.
	gfsdk_U32 BoneNum = 0;
	HairWorksSDK->GetNumBones(HairAssetId, &BoneNum);

	Hair.BoneNames.Empty(BoneNum);

	for (gfsdk_U32 Idx = 0; Idx < BoneNum; ++Idx)
	{
		gfsdk_char BoneName[GFSDK_HAIR_MAX_STRING];
		HairWorksSDK->GetBoneName(HairAssetId, Idx, BoneName);

		Hair.BoneNames.Add(*FixupBoneName(BoneName));
	}

	// Get material
	if (Hair.bMaterials)
	{
		GFSDK_HairInstanceDescriptor HairInstanceDesc;
		if (NewInstanceDesc)
			HairInstanceDesc = *NewInstanceDesc;
		else
			HairWorksSDK->CopyInstanceDescriptorFromAsset(HairAssetId, HairInstanceDesc);

		//TODO: Fix this nonsense
		// sRGB conversion
		auto ConvertColorToSRGB = [](gfsdk_float4& Color)
		{
			reinterpret_cast<FLinearColor&>(Color) = FLinearColor(FColor(Color.x * 255, Color.y * 255, Color.z * 255));
		};

		ConvertColorToSRGB(HairInstanceDesc.m_rootColor);
		ConvertColorToSRGB(HairInstanceDesc.m_tipColor);
		ConvertColorToSRGB(HairInstanceDesc.m_specularColor);

		// Because of SRGB conversion, we need to use a different diffuse blend value to keep consistent with HairWorks Viewer.
		HairInstanceDesc.m_diffuseBlend = 1 - FMath::Pow(1 - HairInstanceDesc.m_diffuseBlend, 2.2f);

		// UE4 shadow attenuation is different from HairWorks viewer, so we use a different value to keep consistent
		HairInstanceDesc.m_shadowSigma /= 2;
		HairInstanceDesc.m_shadowSigma = FMath::Min(HairInstanceDesc.m_shadowSigma, 254.f / 255.f);

		// Fill hair material
		if (Hair.HairMaterial == nullptr)
			Hair.HairMaterial = NewObject<UHairWorksMaterial>(&Hair);

		if (HairInstanceDesc.m_hairNormalBoneIndex < (unsigned)Hair.BoneNames.Num())
			Hair.HairMaterial->HairNormalCenter = Hair.BoneNames[HairInstanceDesc.m_hairNormalBoneIndex];

		TArray<UTexture2D*> HairTextures;
		GFSDK_HairInstanceDescriptor TmpHairInstanceDesc;
		Hair.HairMaterial->SyncHairDescriptor(TmpHairInstanceDesc, HairTextures, false);	// To keep textures.

		Hair.HairMaterial->SyncHairDescriptor(HairInstanceDesc, HairTextures, true);
	}
}