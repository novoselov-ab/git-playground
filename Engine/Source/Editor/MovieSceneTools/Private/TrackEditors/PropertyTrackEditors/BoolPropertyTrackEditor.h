// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "MovieSceneBoolSection.h"
#include "MovieSceneBoolTrack.h"

/**
* A property track editor for bools.
*/
class FBoolPropertyTrackEditor
	: public FPropertyTrackEditor<UMovieSceneBoolTrack, UMovieSceneBoolSection, bool>
{
public:
	/**
	 * Constructor
	 *
	 * @param InSequencer The sequencer instance to be used by this tool
	 */
	FBoolPropertyTrackEditor( TSharedRef<ISequencer> InSequencer )
		: FPropertyTrackEditor( InSequencer, NAME_BoolProperty )
	{ }

	/**
	 * Creates an instance of this class.  Called by a sequencer 
	 *
	 * @param OwningSequencer The sequencer instance to be used by this tool
	 * @return The new instance of this class
	 */
	static TSharedRef<ISequencerTrackEditor> CreateTrackEditor( TSharedRef<ISequencer> OwningSequencer );

protected:

	// FPropertyTrackEditor interface
	virtual TSharedRef<FPropertySection> MakePropertySectionInterface( UMovieSceneSection& SectionObject, UMovieSceneTrack& Track ) override;
	virtual void GenerateKeysFromPropertyChanged( const FPropertyChangedParams& PropertyChangedParams, TArray<bool>& NewGeneratedKeys, TArray<bool>& DefaultGeneratedKeys ) override;
};
