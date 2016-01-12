// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "MovieSceneTracksPrivatePCH.h"
#include "MovieScene3DTransformSection.h"
#include "MovieScene3DTransformTrack.h"
#include "IMovieScenePlayer.h"
#include "MovieScene3DTransformTrackInstance.h"


bool FTransformKey::ShouldKeyTranslation( EAxis::Type Axis) const
{
	// If the previous transform is not valid we have nothing to compare against so assume we always key. Otherwise check for differences
	return !PreviousTransform.IsValid() || GetVectorComponentIfDifferent( Axis, NewTransform.Translation, PreviousTransform.Translation );
}


bool FTransformKey::ShouldKeyRotation( EAxis::Type Axis ) const
{
	// If the previous transform is not valid we have nothing to compare against so assume we always key. Otherwise check for differences
	return !PreviousTransform.IsValid() || GetVectorComponentIfDifferent( Axis, NewTransform.Rotation.Euler(), PreviousTransform.Rotation.Euler() );
}


bool FTransformKey::ShouldKeyScale( EAxis::Type Axis ) const
{
	// If the previous transform is not valid we have nothing to compare against so assume we always key. Otherwise check for differences
	return !PreviousTransform.IsValid() || GetVectorComponentIfDifferent( Axis, NewTransform.Scale, PreviousTransform.Scale );
}


bool FTransformKey::ShouldKeyAny() const
{
	return ShouldKeyTranslation(EAxis::X) || ShouldKeyTranslation(EAxis::Y) || ShouldKeyTranslation(EAxis::Z) ||
		ShouldKeyRotation(EAxis::X) || ShouldKeyRotation(EAxis::Y) || ShouldKeyRotation(EAxis::Z) ||
		ShouldKeyScale(EAxis::X) || ShouldKeyScale(EAxis::Y) || ShouldKeyScale(EAxis::Z);
}


bool FTransformKey::GetVectorComponentIfDifferent( EAxis::Type Axis, const FVector& Current, const FVector& Previous ) const
{
	bool bShouldAddKey = false;

	// Using lower precision to avoid small changes due to floating point error
	// @todo Sequencer - Floating point error introduced by matrix math causing extra keys to be added
	const float Precision = 1e-3;

	// Add a key if there is no previous transform to compare against or the current and previous values are not equal

	if( Axis == EAxis::X && !FMath::IsNearlyEqual( Current.X, Previous.X, Precision ) )
	{
		bShouldAddKey = true;
	}
	else if( Axis == EAxis::Y && !FMath::IsNearlyEqual( Current.Y, Previous.Y, Precision ) )
	{
		bShouldAddKey = true;
	}
	else if( Axis == EAxis::Z && !FMath::IsNearlyEqual( Current.Z, Previous.Z, Precision ) )
	{
		bShouldAddKey = true;
	}

	return bShouldAddKey;
}


UMovieScene3DTransformTrack::UMovieScene3DTransformTrack( const FObjectInitializer& ObjectInitializer )
	: Super( ObjectInitializer )
{ }


UMovieSceneSection* UMovieScene3DTransformTrack::CreateNewSection()
{
	return NewObject<UMovieSceneSection>(this, UMovieScene3DTransformSection::StaticClass(), NAME_None, RF_Transactional);
}


TSharedPtr<IMovieSceneTrackInstance> UMovieScene3DTransformTrack::CreateInstance()
{
	return MakeShareable( new FMovieScene3DTransformTrackInstance( *this ) );
}


bool UMovieScene3DTransformTrack::AddKeyToSection( const FGuid& ObjectHandle, const FTransformKey& InKey, const bool bUnwindRotation, F3DTransformTrackKey::Type KeyType )
{
	const UMovieSceneSection* NearestSection = MovieSceneHelpers::FindNearestSectionAtTime(Sections, InKey.GetKeyTime());
	if (!NearestSection || InKey.KeyParams.bAddKeyEvenIfUnchanged || CastChecked<UMovieScene3DTransformSection>(NearestSection)->NewKeyIsNewData(InKey))
	{
		Modify();

		UMovieScene3DTransformSection* NewSection = Cast<UMovieScene3DTransformSection>( FindOrAddSection( InKey.GetKeyTime() ) );

		FTransformKey Key(InKey);

		Key.KeyParams.bAddKeyEvenIfUnchanged = InKey.KeyParams.bAddKeyEvenIfUnchanged && (KeyType & F3DTransformTrackKey::Key_Translation);
		NewSection->AddTranslationKeys( Key );

		Key.KeyParams.bAddKeyEvenIfUnchanged = InKey.KeyParams.bAddKeyEvenIfUnchanged && (KeyType & F3DTransformTrackKey::Key_Rotation);
		NewSection->AddRotationKeys( Key, bUnwindRotation );

		Key.KeyParams.bAddKeyEvenIfUnchanged = InKey.KeyParams.bAddKeyEvenIfUnchanged && (KeyType & F3DTransformTrackKey::Key_Scale);
		NewSection->AddScaleKeys( Key );

		return true;
	}
	return false;
}


bool UMovieScene3DTransformTrack::Eval( float Position, float LastPosition, FVector& OutTranslation, FRotator& OutRotation, FVector& OutScale ) const
{
	const UMovieSceneSection* Section = MovieSceneHelpers::FindNearestSectionAtTime( Sections, Position );

	if( Section )
	{
		const UMovieScene3DTransformSection* TransformSection = CastChecked<UMovieScene3DTransformSection>( Section );

		if (!Section->IsInfinite())
		{
			Position = FMath::Clamp(Position, Section->GetStartTime(), Section->GetEndTime());
		}

		// Evaluate translation,rotation, and scale curves.  If no keys were found on one of these, that component of the transform will remain unchained
		TransformSection->EvalTranslation( Position, OutTranslation );
		TransformSection->EvalRotation( Position, OutRotation );
		TransformSection->EvalScale( Position, OutScale );
	}

	return Section != nullptr;
}


bool UMovieScene3DTransformTrack::CanKeyTrack(const FTransformKey& InKey ) const
{
	const UMovieSceneSection* NearestSection = MovieSceneHelpers::FindNearestSectionAtTime(Sections, InKey.GetKeyTime());
	if (!NearestSection || CastChecked<UMovieScene3DTransformSection>(NearestSection)->NewKeyIsNewData(InKey))
	{
		return true;
	}
	return false;
}
