#include "HairworksPCH.h"

UHairWorksComponent::UHairWorksComponent(const FObjectInitializer& PCIP)
	: Super(PCIP)
{
	// No need to select
	bSelectable = false;

	// Simplify shadow
	CastShadow = true;
	bAffectDynamicIndirectLighting = false;
	bAffectDistanceFieldLighting = false;

	// Setup tick
	bAutoActivate = true;
	bTickInEditor = true;
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostUpdateWork;	// Just mark render data dirty every frame

	UE_LOG(LogHairWorks, Log, TEXT("Creating new HairComponent"));
}

UHairWorksComponent::~UHairWorksComponent()
{
}

FPrimitiveSceneProxy* UHairWorksComponent::CreateSceneProxy()
{
	auto owner = this->GetOwner();

	// Call this to make sure the sim timer is ticking
	GHairManager->EnsureHairSetup();

	UE_LOG(LogHairWorks, Log, TEXT("HC:CreateSceneProxy for Hair %d, parent %d"), GetUniqueID(), owner->GetUniqueID());

	if (HairInstance.Hair == nullptr)
		return nullptr;

	if (HairInstance.Hair->AssetId == GFSDK_HairAssetID_NULL)
	{
		if (!HairInstance.Hair->LoadHairAsset())
		{
			return nullptr;
		}
	}

	check(HairInstance.Hair->AssetId != GFSDK_HairAssetID_NULL);

	SceneProxy = new FHairWorksSceneProxy(this, HairInstance.Hair->AssetId);

	return SceneProxy;
}

void UHairWorksComponent::OnAttachmentChanged()
{
	Super::OnAttachmentChanged();

	// Update proxy
	SetupBoneMapping();
	SendHairDynamicData();	// For correct initial visual effect
}

FBoxSphereBounds UHairWorksComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	if (!SceneProxy)
		return FBoxSphereBounds(EForceInit::ForceInit);

	auto HairProxy = static_cast<FHairWorksSceneProxy*>(SceneProxy);

	gfsdk_float3 Min;
	gfsdk_float3 Max;

	if (GHairManager->GetHairworksSdk()->GetBounds(HairProxy->getHairInstanceID(), &Min, &Max) != GFSDK_HAIR_RETURN_OK)
		return FBoxSphereBounds(EForceInit::ForceInitToZero);

	auto result = FBoxSphereBounds(FBox(FVector(Min.x, Min.y, Min.z), FVector(Max.x, Max.y, Max.z)));

	result = result.TransformBy(LocalToWorld);

//	UE_LOG(LogHairWorks, Log, TEXT("ID: %s HW Bounds: %s"), *GetName(), *result.ToString());

	return result;
}


#if WITH_EDITOR
static FName Name_HairInstance = GET_MEMBER_NAME_CHECKED(UHairWorksComponent, HairInstance);
static FName Name_Hair = GET_MEMBER_NAME_CHECKED(FHairWorksInstance, Hair);

#pragma optimize("", off)

void UHairWorksComponent::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);

	FName MemberPropertyName = PropertyChangedEvent.PropertyChain.GetActiveMemberNode()->GetValue()->GetFName();

	if (MemberPropertyName != NAME_None)
	{
		UProperty* PropertyThatChanged = PropertyChangedEvent.Property;
		FName PropertyName = PropertyThatChanged != NULL ? PropertyThatChanged->GetFName() : NAME_None;

		if (PropertyName == Name_Hair)
		{
			for (TFieldIterator<UProperty> PropIt(UHairWorksMaterial::StaticClass()); PropIt; ++PropIt)
			{
				auto Property = *PropIt;
				Property->CopyCompleteValue_InContainer(HairInstance.HairMaterial, HairInstance.Hair->HairMaterial);
			}
		}
	}
}

#endif

void UHairWorksComponent::SendRenderDynamicData_Concurrent()
{
	Super::SendRenderDynamicData_Concurrent();

	// Send data for rendering
	SendHairDynamicData();
}

void UHairWorksComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Send data every frame
	if (SceneProxy)
	{
		//JDM: Not sure this is appropriate.
#if WITH_EDITOR
		if (!(GetWorld() && GetWorld()->bPostTickComponentUpdate))
			MarkRenderTransformDirty();	// Update scene cached bounds.
#endif

		MarkRenderDynamicDataDirty();
	}
}

void UHairWorksComponent::CreateRenderState_Concurrent()
{
	Super::CreateRenderState_Concurrent();

	UE_LOG(LogHairWorks, Log, TEXT("HC:CreateRenderState_C"));

	//JDM: Is this required?
//	FlushRenderingCommands();	// Ensure hair is created.

	UpdateBounds();

	if (!(GetWorld() && GetWorld()->bPostTickComponentUpdate))
		MarkRenderTransformDirty();	// Update scene cached bounds.

	// Update proxy
	SetupBoneMapping();
	SendHairDynamicData();	// Ensure correct visual effect at first frame.
}

void UHairWorksComponent::SendHairDynamicData()
{
	// Update parameters
	if (!SceneProxy)
		return;


	auto ParentSkeleton = Cast<USkinnedMeshComponent>(AttachParent);

	if (ParentSkeleton == nullptr)
	{
		// No parent? Don't do any of this.
		return;
	}

	TSharedPtr<FHairWorksSceneProxy::FDynamicRenderData> DynamicData = MakeShareable<FHairWorksSceneProxy::FDynamicRenderData>(new FHairWorksSceneProxy::FDynamicRenderData());

	if (ParentSkeleton != nullptr && ParentSkeleton->SkeletalMesh != nullptr)
	{
		DynamicData->BoneMatrices.SetNum(BoneIndices.Num());

		for (auto Idx = 0; Idx < BoneIndices.Num(); ++Idx)
		{
			auto IdxInParent = BoneIndices[Idx];

			auto Matrix = ParentSkeleton->GetSpaceBases()[IdxInParent].ToMatrixWithScale();

			DynamicData->BoneMatrices[Idx] = ParentSkeleton->SkeletalMesh->RefBasesInvMatrix[IdxInParent] * Matrix;
		}
	}

	int numBones = DynamicData->BoneMatrices.Num();

	// Setup material
	DynamicData->Textures.SetNumZeroed(GFSDK_HAIR_NUM_TEXTURES);
	DynamicData->NormalCenterBoneIndex = -1;

	if (HairInstance.Hair->HairMaterial != nullptr)	// Always load from asset to propagate visualization flags.
	{
		// Populate DynamicData->HairInstanceDesc with the data from the assets HairMaterial
		HairInstance.Hair->HairMaterial->SyncHairDescriptor(DynamicData->HairInstanceDesc, DynamicData->Textures, false);

		// Find the HairNormalCenter bone in the index map
		auto boneIdxPtr = HairBoneToIdxMap.Find(HairInstance.Hair->HairMaterial->HairNormalCenter);
		
		if (boneIdxPtr != nullptr)
		{
			DynamicData->NormalCenterBoneIndex = *boneIdxPtr;
		}
	}

	if (HairInstance.HairMaterial != nullptr && HairInstance.bOverride)
	{
		// Populate DynamicData->HairInstanceDesc with the data from the overriding HairMaterial
		HairInstance.HairMaterial->SyncHairDescriptor(DynamicData->HairInstanceDesc, DynamicData->Textures, false);

		// Find the HairNormalCenter bone in the index map
		auto boneIdxPtr = HairBoneToIdxMap.Find(HairInstance.HairMaterial->HairNormalCenter);

		if (boneIdxPtr != nullptr)
		{
			DynamicData->NormalCenterBoneIndex = *boneIdxPtr;
		}
	}

	auto HairSceneProxy = static_cast<FHairWorksSceneProxy*>(SceneProxy);

	ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
		HairUpdateDynamicData,
		FHairWorksSceneProxy*, ThisProxy, HairSceneProxy,
		TSharedPtr<FHairWorksSceneProxy::FDynamicRenderData>, DynamicData, DynamicData,
		{
			ThisProxy->UpdateDynamicData_RenderThread(DynamicData);
		}
	);
}

void UHairWorksComponent::SetupBoneMapping()
{
	auto ParentSkeleton = Cast<USkinnedMeshComponent>(AttachParent);

	if (HairInstance.Hair == nullptr || ParentSkeleton == nullptr || ParentSkeleton->SkeletalMesh == nullptr)
		return;

	auto Bones = ParentSkeleton->SkeletalMesh->RefSkeleton.GetRefBoneInfo();
	BoneIndices.SetNumUninitialized(HairInstance.Hair->BoneNames.Num());

	for (auto Idx = 0; Idx < BoneIndices.Num(); ++Idx)
	{
		BoneIndices[Idx] = Bones.IndexOfByPredicate([&](const FMeshBoneInfo& BoneInfo){return BoneInfo.Name == HairInstance.Hair->BoneNames[Idx]; });
	}
}

