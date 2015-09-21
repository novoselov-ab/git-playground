#include "HairworksPCH.h"

UHairComponent::UHairComponent(const class FObjectInitializer& PCIP)
	: Super(PCIP),
	Hair(nullptr),
	ParentSkeleton(nullptr)
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

	// Set properties' default values
	// JDM: FIXME
// 	GFSDK_HairInstanceDescriptor HairDescriptor;
// 	TArray<FTexture2DRHIRef> HairTextures;
// 	SyncHairParameters(HairDescriptor, HairTextures, true);

	UE_LOG(LogHairWorks, Log, TEXT("Creating new HairComponent"));
}

UHairComponent::~UHairComponent()
{
}

FPrimitiveSceneProxy* UHairComponent::CreateSceneProxy()
{
	if (!Hair)
		return nullptr;

	auto owner = this->GetOwner();

	UE_LOG(LogHairWorks, Log, TEXT("HC:CreateSceneProxy for Hair %d, parent %d"), GetUniqueID(), owner->GetUniqueID());

	if (Hair->AssetId == GFSDK_HairAssetID_NULL)
	{
		if (!Hair->LoadHairAsset())
		{
			return nullptr;
		}
	}

	check(Hair->AssetId != GFSDK_HairAssetID_NULL);


	return new FHairSceneProxy(this, Hair->AssetId);
}

void UHairComponent::OnAttachmentChanged()
{
	Super::OnAttachmentChanged();

	// Parent as skeleton
	ParentSkeleton = Cast<USkinnedMeshComponent>(AttachParent);

	// Update proxy
	SetupBoneMapping();
	SendHairDynamicData();	// For correct initial visual effect
}

FBoxSphereBounds UHairComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	if (!SceneProxy)
		return FBoxSphereBounds(EForceInit::ForceInit);

	auto HairProxy = static_cast<FHairSceneProxy*>(SceneProxy);

	FBoxSphereBounds HairBounds(EForceInit::ForceInitToZero);
	if (HairProxy->GetHairBounds_GameThread(HairBounds))
	{
		return HairBounds.TransformBy(LocalToWorld);
	}

	return HairBounds;
}


#if WITH_EDITOR
static FName Name_Hair = GET_MEMBER_NAME_CHECKED(UHairComponent, Hair);
static FName Name_HairProperties = GET_MEMBER_NAME_CHECKED(UHairComponent, HairProperties);

void UHairComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	UProperty* PropertyThatChanged = PropertyChangedEvent.Property;
	FName PropertyName = PropertyThatChanged != NULL ? PropertyThatChanged->GetFName() : NAME_None;

	if (PropertyName != NAME_None)
	{
		if (PropertyName == Name_Hair)
		{
			// If Hair changed, do a simple copy of the properties
			if (Hair != nullptr)
			{
				HairProperties = Hair->HairProperties;
			}
		}
		else if (PropertyName == Name_HairProperties)
		{
			if (Hair != nullptr)
			{
				HairProperties.ExportPropertiesTo(CachedHairDescriptor);

				if (SceneProxy)
				{
					SendHairDynamicData();
				}
			}
		}
	}
}
#endif

void UHairComponent::SendRenderDynamicData_Concurrent()
{
	Super::SendRenderDynamicData_Concurrent();

	// Send data for rendering
	SendHairDynamicData();
}

void UHairComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
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

void UHairComponent::CreateRenderState_Concurrent()
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

void UHairComponent::SendHairDynamicData()
{
	// Update parameters
	if (!SceneProxy)
		return;

	auto HairSceneProxy = static_cast<FHairSceneProxy*>(SceneProxy);

	if (ParentSkeleton)
		HairSceneProxy->UpdateBones_GameThread(*ParentSkeleton);

	// Send paramters
	HairSceneProxy->UpdateHairParams_GameThread(CachedHairDescriptor, HairProperties.GetTextures());
}

void UHairComponent::SetupBoneMapping()
{
	// Setup bone mapping
	if (!SceneProxy || !ParentSkeleton || !ParentSkeleton->SkeletalMesh)
		return;

	//JDM: FIXME - probably doesn't need to happen in the render thread.
	auto HairSceneProxy = static_cast<FHairSceneProxy*>(SceneProxy);
//	HairSceneProxy->SetupBoneMapping_RenderThread(ParentSkeleton->SkeletalMesh->RefSkeleton.GetRefBoneInfo());
	HairSceneProxy->SetupBoneMapping_GameThread(ParentSkeleton->SkeletalMesh->RefSkeleton.GetRefBoneInfo());
}

