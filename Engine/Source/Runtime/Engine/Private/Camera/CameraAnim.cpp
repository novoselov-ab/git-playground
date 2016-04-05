// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "EnginePrivate.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraAnim.h"
#include "ParticleDefinitions.h"
#include "Camera/CameraActor.h"
#include "Matinee/MatineeActor.h"
#include "Matinee/InterpData.h"
#include "Matinee/InterpGroup.h"
#include "Matinee/InterpGroupInst.h"
#include "Matinee/InterpGroupCamera.h"
#include "Matinee/InterpTrackMove.h"


//////////////////////////////////////////////////////////////////////////
// UCameraAnim

UCameraAnim::UCameraAnim(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AnimLength = 3.0f;
	bRelativeToInitialTransform = true;
	BaseFOV = 90.0f;
}


bool UCameraAnim::CreateFromInterpGroup(class UInterpGroup* SrcGroup, class AMatineeActor* InMatineeActor)
{
	// assert we're controlling a camera actor
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	{
		UInterpGroupInst* GroupInst = InMatineeActor ? InMatineeActor->FindFirstGroupInst(SrcGroup) : NULL;
		if (GroupInst)
		{
			check( GroupInst->GetGroupActor()->IsA(ACameraActor::StaticClass()) );
		}
	}
#endif
	
	// copy length information
	AnimLength = (InMatineeActor && InMatineeActor->MatineeData) ? InMatineeActor->MatineeData->InterpLength : 0.f;

	UInterpGroup* OldGroup = CameraInterpGroup;

	if (CameraInterpGroup != SrcGroup)
	{
		// copy the source interp group for use in the CameraAnim
		// @fixme jf: fixed this potentially creating an object of UInterpGroup and raw casting it to InterpGroupCamera.  No source data in UE4 to test though.
		CameraInterpGroup = Cast<UInterpGroupCamera>(StaticDuplicateObject(SrcGroup, this, NAME_None, RF_AllFlags, UInterpGroupCamera::StaticClass()));

		if (CameraInterpGroup)
		{
			// delete the old one, if it exists
			if (OldGroup)
			{
				OldGroup->MarkPendingKill();
			}

			// success!
			return true;
		}
		else
		{
			// creation of new one failed somehow, restore the old one
			CameraInterpGroup = OldGroup;
		}
	}
	else
	{
		// no need to perform work above, but still a "success" case
		return true;
	}

	// failed creation
	return false;
}


FBox UCameraAnim::GetAABB(FVector const& BaseLoc, FRotator const& BaseRot, float Scale) const
{
	FRotationTranslationMatrix const BaseTM(BaseRot, BaseLoc);

	FBox ScaledLocalBox = BoundingBox;
	ScaledLocalBox.Min *= Scale;
	ScaledLocalBox.Max *= Scale;

	return ScaledLocalBox.TransformBy(BaseTM);
}


void UCameraAnim::PreSave()
{
#if WITH_EDITORONLY_DATA
	CalcLocalAABB();
#endif // WITH_EDITORONLY_DATA
	Super::PreSave();
}

void UCameraAnim::PostLoad()
{
	if (GIsEditor)
	{
		// update existing CameraAnims' bboxes on load, so editor knows they 
		// they need to be resaved
		if (!BoundingBox.IsValid)
		{
			CalcLocalAABB();
			if (BoundingBox.IsValid)
			{
				MarkPackageDirty();
			}
		}
	}

	Super::PostLoad();
}	


void UCameraAnim::CalcLocalAABB()
{
	BoundingBox.Init();

	if (CameraInterpGroup)
	{
		// find move track
		UInterpTrackMove *MoveTrack = NULL;
		for (int32 TrackIdx = 0; TrackIdx < CameraInterpGroup->InterpTracks.Num(); ++TrackIdx)
		{
			MoveTrack = Cast<UInterpTrackMove>(CameraInterpGroup->InterpTracks[TrackIdx]);
			if (MoveTrack != NULL)
			{
				break;
			}
		}

		if (MoveTrack != NULL)
		{
			FVector Zero(0.f), MinBounds, MaxBounds;
			MoveTrack->PosTrack.CalcBounds(MinBounds, MaxBounds, Zero);
			BoundingBox = FBox(MinBounds, MaxBounds);
		}
	}
}

SIZE_T UCameraAnim::GetResourceSize(EResourceSizeMode::Type Mode)
{
	int32 ResourceSize = 0;

	if (Mode == EResourceSizeMode::Inclusive && CameraInterpGroup)
	{
		// find move track
		UInterpTrackMove *MoveTrack = NULL;
		for (int32 TrackIdx = 0; TrackIdx < CameraInterpGroup->InterpTracks.Num(); ++TrackIdx)
		{
			// somehow movement track's not calculated when you just used serialize, so I'm adding it here. 
			MoveTrack = Cast<UInterpTrackMove>(CameraInterpGroup->InterpTracks[TrackIdx]);
			if (MoveTrack)
			{
				FArchiveCountMem CountBytesSize(MoveTrack);
				ResourceSize += CountBytesSize.GetNum();
			}
		}
	}
	return ResourceSize;
}
