// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "MovieSceneToolsPrivatePCH.h"
#include "PropertyEditing.h"
#include "MovieScene.h"
#include "MovieSceneTrack.h"
#include "ScopedTransaction.h"
#include "MovieSceneSection.h"
#include "ISequencerObjectChangeListener.h"
#include "ISequencerSection.h"
#include "ISectionLayoutBuilder.h"
#include "IKeyArea.h"
#include "MovieSceneToolHelpers.h"
#include "SFloatCurveKeyEditor.h"
#include "SIntegralCurveKeyEditor.h"
#include "SEnumCurveKeyEditor.h"
#include "SBoolCurveKeyEditor.h"


TArray<FKeyHandle> FFloatCurveKeyArea::AddKeyUnique(float Time, EMovieSceneKeyInterpolation InKeyInterpolation, float TimeToCopyFrom)
{
	TArray<FKeyHandle> AddedKeyHandles;

	FKeyHandle CurrentKeyHandle = Curve->FindKey(Time);
	if (Curve->IsKeyHandleValid(CurrentKeyHandle) == false)
	{
		if (OwningSection->GetStartTime() > Time)
		{
			OwningSection->SetStartTime(Time);
		}
		if (OwningSection->GetEndTime() < Time)
		{
			OwningSection->SetEndTime(Time);
		}

		float Value = Curve->Eval(Time);
		if (TimeToCopyFrom != FLT_MAX)
		{
			Value = Curve->Eval(TimeToCopyFrom);
		}

		Curve->AddKey(Time, Value, false, CurrentKeyHandle);
		AddedKeyHandles.Add(CurrentKeyHandle);
		
		// Set the default key interpolation
		switch (InKeyInterpolation)
		{
			case MSKI_Auto:
				Curve->SetKeyInterpMode(CurrentKeyHandle, RCIM_Cubic);
				Curve->SetKeyTangentMode(CurrentKeyHandle, RCTM_Auto);
				break;
			case MSKI_User:
				Curve->SetKeyInterpMode(CurrentKeyHandle, RCIM_Cubic);
				Curve->SetKeyTangentMode(CurrentKeyHandle, RCTM_User);
				break;
			case MSKI_Break:
				Curve->SetKeyInterpMode(CurrentKeyHandle, RCIM_Cubic);
				Curve->SetKeyTangentMode(CurrentKeyHandle, RCTM_Break);
				break;
			case MSKI_Linear:
				Curve->SetKeyInterpMode(CurrentKeyHandle, RCIM_Linear);
				Curve->SetKeyTangentMode(CurrentKeyHandle, RCTM_Auto);
				break;
			case MSKI_Constant:
				Curve->SetKeyInterpMode(CurrentKeyHandle, RCIM_Constant);
				Curve->SetKeyTangentMode(CurrentKeyHandle, RCTM_Auto);
				break;
			default:
				Curve->SetKeyInterpMode(CurrentKeyHandle, RCIM_Cubic);
				Curve->SetKeyTangentMode(CurrentKeyHandle, RCTM_Auto);
				break;
		}
		
		// Copy the properties from the key if it exists
		FKeyHandle KeyHandleToCopy = Curve->FindKey(TimeToCopyFrom);
		if (Curve->IsKeyHandleValid(KeyHandleToCopy))
		{
			FRichCurveKey& CurrentKey = Curve->GetKey(CurrentKeyHandle);
			FRichCurveKey& KeyToCopy = Curve->GetKey(KeyHandleToCopy);
			CurrentKey.InterpMode = KeyToCopy.InterpMode;
			CurrentKey.TangentMode = KeyToCopy.TangentMode;
			CurrentKey.TangentWeightMode = KeyToCopy.TangentWeightMode;
			CurrentKey.ArriveTangent = KeyToCopy.ArriveTangent;
			CurrentKey.LeaveTangent = KeyToCopy.LeaveTangent;
			CurrentKey.ArriveTangentWeight = KeyToCopy.ArriveTangentWeight;
			CurrentKey.LeaveTangentWeight = KeyToCopy.LeaveTangentWeight;
		}
	}

	return AddedKeyHandles;
}


TSharedRef<SWidget> FFloatCurveKeyArea::CreateKeyEditor(ISequencer* Sequencer)
{
	return SNew(SFloatCurveKeyEditor)
		.Sequencer(Sequencer)
		.OwningSection(OwningSection)
		.Curve(Curve);
};


TArray<FKeyHandle> FNameCurveKeyArea::AddKeyUnique(float Time, EMovieSceneKeyInterpolation InKeyInterpolation, float TimeToCopyFrom)
{
	TArray<FKeyHandle> AddedKeyHandles;
	FKeyHandle CurrentKey = Curve.FindKey(Time);

	if (Curve.IsKeyHandleValid(CurrentKey) == false)
	{
		if (OwningSection->GetStartTime() > Time)
		{
			OwningSection->SetStartTime(Time);
		}

		if (OwningSection->GetEndTime() < Time)
		{
			OwningSection->SetEndTime(Time);
		}

		Curve.AddKey(Time, NAME_None, CurrentKey);
		AddedKeyHandles.Add(CurrentKey);
	}

	return AddedKeyHandles;
}


TArray<FKeyHandle> FIntegralKeyArea::AddKeyUnique(float Time, EMovieSceneKeyInterpolation InKeyInterpolation, float TimeToCopyFrom)
{
	TArray<FKeyHandle> AddedKeyHandles;

	FKeyHandle CurrentKey = Curve.FindKey(Time);
	if (Curve.IsKeyHandleValid(CurrentKey) == false)
	{
		if (OwningSection->GetStartTime() > Time)
		{
			OwningSection->SetStartTime(Time);
		}
		if (OwningSection->GetEndTime() < Time)
		{
			OwningSection->SetEndTime(Time);
		}

		int32 Value = Curve.Evaluate(Time);
		if (TimeToCopyFrom != FLT_MAX)
		{
			Value = Curve.Evaluate(TimeToCopyFrom);
		}

		Curve.AddKey(Time, Value, CurrentKey);
		AddedKeyHandles.Add(CurrentKey);
	}

	return AddedKeyHandles;
}


TSharedRef<SWidget> FIntegralKeyArea::CreateKeyEditor(ISequencer* Sequencer)
{
	return SNew(SIntegralCurveKeyEditor)
		.Sequencer(Sequencer)
		.OwningSection(OwningSection)
		.Curve(&Curve);
};


TSharedRef<SWidget> FEnumKeyArea::CreateKeyEditor(ISequencer* Sequencer)
{
	return SNew(SEnumCurveKeyEditor)
		.Sequencer(Sequencer)
		.OwningSection(OwningSection)
		.Curve(&Curve)
		.Enum(Enum);
};


TSharedRef<SWidget> FBoolKeyArea::CreateKeyEditor(ISequencer* Sequencer)
{
	return SNew(SBoolCurveKeyEditor)
		.Sequencer(Sequencer)
		.OwningSection(OwningSection)
		.Curve(&Curve);
};


void MovieSceneToolHelpers::TrimSection(const TSet<TWeakObjectPtr<UMovieSceneSection>>& Sections, float Time, bool bTrimLeft)
{
	for (auto Section : Sections)
	{
		if (Section.IsValid() && Section->IsTimeWithinSection(Time))
		{
			Section->SetFlags( RF_Transactional );

			Section->Modify();

			if (bTrimLeft)
			{
				Section->SetStartTime(Time);
			}
			else
			{
				Section->SetEndTime(Time);
			}
		}
	}
}


void MovieSceneToolHelpers::SplitSection(const TSet<TWeakObjectPtr<UMovieSceneSection>>& Sections, float Time)
{
	for (auto Section : Sections)
	{
		if (Section.IsValid() && Section->IsTimeWithinSection(Time))
		{
			Section->SetFlags( RF_Transactional );

			Section->Modify();

			float SectionEndTime = Section->GetEndTime();
				
			// Trim off the right
			Section->SetEndTime(Time);

			// Create a new section
			UMovieSceneTrack* Track = CastChecked<UMovieSceneTrack>(Section->GetOuter() );
			Track->Modify();

			UMovieSceneSection* NewSection = DuplicateObject<UMovieSceneSection>(Section.Get(), Track);
			Track->AddSection(NewSection);
			NewSection->SetStartTime(Time);
			NewSection->SetEndTime(SectionEndTime);
		}
	}
}
