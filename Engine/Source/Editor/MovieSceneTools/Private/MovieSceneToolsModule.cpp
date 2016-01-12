// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "MovieSceneToolsPrivatePCH.h"
#include "ModuleManager.h"
#include "ISequencerModule.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "MovieSceneSection.h"
#include "ISequencerSection.h"
#include "ScopedTransaction.h"
#include "MovieScene.h"
#include "MovieSceneTrackEditor.h"

#include "BoolPropertyTrackEditor.h"
#include "BytePropertyTrackEditor.h"
#include "ColorPropertyTrackEditor.h"
#include "FloatPropertyTrackEditor.h"
#include "VectorPropertyTrackEditor.h"
#include "VisibilityPropertyTrackEditor.h"

#include "TransformTrackEditor.h"
#include "ShotTrackEditor.h"
#include "SlomoTrackEditor.h"
#include "SubMovieSceneTrackEditor.h"
#include "AudioTrackEditor.h"
#include "SkeletalAnimationTrackEditor.h"
#include "ParticleTrackEditor.h"
#include "AttachTrackEditor.h"
#include "EventTrackEditor.h"
#include "PathTrackEditor.h"
#include "MaterialTrackEditor.h"
#include "FadeTrackEditor.h"


/**
 * Implements the MovieSceneTools module.
 */
class FMovieSceneToolsModule
	: public IMovieSceneTools
{
public:

	// IModuleInterface interface

	virtual void StartupModule() override
	{
		ISequencerModule& SequencerModule = FModuleManager::Get().LoadModuleChecked<ISequencerModule>( "Sequencer" );

		// register property track editors
		BoolPropertyTrackCreateEditorHandle = SequencerModule.RegisterTrackEditor_Handle( FOnCreateTrackEditor::CreateStatic( &FBoolPropertyTrackEditor::CreateTrackEditor ) );
		BytePropertyTrackCreateEditorHandle = SequencerModule.RegisterTrackEditor_Handle( FOnCreateTrackEditor::CreateStatic( &FBytePropertyTrackEditor::CreateTrackEditor ) );
		ColorPropertyTrackCreateEditorHandle = SequencerModule.RegisterTrackEditor_Handle( FOnCreateTrackEditor::CreateStatic( &FColorPropertyTrackEditor::CreateTrackEditor ) );
		FloatPropertyTrackCreateEditorHandle = SequencerModule.RegisterTrackEditor_Handle( FOnCreateTrackEditor::CreateStatic( &FFloatPropertyTrackEditor::CreateTrackEditor ) );
		VectorPropertyTrackCreateEditorHandle = SequencerModule.RegisterTrackEditor_Handle( FOnCreateTrackEditor::CreateStatic( &FVectorPropertyTrackEditor::CreateTrackEditor ) );
		VisibilityPropertyTrackCreateEditorHandle = SequencerModule.RegisterTrackEditor_Handle( FOnCreateTrackEditor::CreateStatic( &FVisibilityPropertyTrackEditor::CreateTrackEditor ) );

		// register specialty track editors
		AnimationTrackCreateEditorHandle = SequencerModule.RegisterTrackEditor_Handle( FOnCreateTrackEditor::CreateStatic( &FSkeletalAnimationTrackEditor::CreateTrackEditor ) );
		AttachTrackCreateEditorHandle = SequencerModule.RegisterTrackEditor_Handle( FOnCreateTrackEditor::CreateStatic( &F3DAttachTrackEditor::CreateTrackEditor ) );
		AudioTrackCreateEditorHandle = SequencerModule.RegisterTrackEditor_Handle( FOnCreateTrackEditor::CreateStatic( &FAudioTrackEditor::CreateTrackEditor ) );
		EventTrackCreateEditorHandle = SequencerModule.RegisterTrackEditor_Handle( FOnCreateTrackEditor::CreateStatic( &FEventTrackEditor::CreateTrackEditor ) );
		ParticleTrackCreateEditorHandle = SequencerModule.RegisterTrackEditor_Handle( FOnCreateTrackEditor::CreateStatic( &FParticleTrackEditor::CreateTrackEditor ) );
		PathTrackCreateEditorHandle = SequencerModule.RegisterTrackEditor_Handle( FOnCreateTrackEditor::CreateStatic( &F3DPathTrackEditor::CreateTrackEditor ) );

		//@todo Disable Shot Track Editor until it's useable
		//ShotTrackCreateEditorHandle = SequencerModule.RegisterTrackEditor_Handle( FOnCreateTrackEditor::CreateStatic( &FShotTrackEditor::CreateTrackEditor ) );

		SlomoTrackCreateEditorHandle = SequencerModule.RegisterTrackEditor_Handle( FOnCreateTrackEditor::CreateStatic( &FSlomoTrackEditor::CreateTrackEditor ) );
		SubMovieSceneTrackCreateEditorHandle = SequencerModule.RegisterTrackEditor_Handle( FOnCreateTrackEditor::CreateStatic( &FSubMovieSceneTrackEditor::CreateTrackEditor ) );
		TransformTrackCreateEditorHandle = SequencerModule.RegisterTrackEditor_Handle( FOnCreateTrackEditor::CreateStatic( &F3DTransformTrackEditor::CreateTrackEditor ) );
		ComponentMaterialTrackCreatEditorHandle = SequencerModule.RegisterTrackEditor_Handle( FOnCreateTrackEditor::CreateStatic( &FComponentMaterialTrackEditor::CreateTrackEditor ) );
		FadeTrackCreatEditorHandle = SequencerModule.RegisterTrackEditor_Handle( FOnCreateTrackEditor::CreateStatic( &FFadeTrackEditor::CreateTrackEditor ) );
	}

	virtual void ShutdownModule() override
	{
		if (!FModuleManager::Get().IsModuleLoaded("Sequencer"))
		{
			return;
		}

		ISequencerModule& SequencerModule = FModuleManager::Get().GetModuleChecked<ISequencerModule>( "Sequencer" );

		// unregister property track editors
		SequencerModule.UnRegisterTrackEditor_Handle( BoolPropertyTrackCreateEditorHandle );
		SequencerModule.UnRegisterTrackEditor_Handle( BytePropertyTrackCreateEditorHandle );
		SequencerModule.UnRegisterTrackEditor_Handle( ColorPropertyTrackCreateEditorHandle );
		SequencerModule.UnRegisterTrackEditor_Handle( FloatPropertyTrackCreateEditorHandle );
		SequencerModule.UnRegisterTrackEditor_Handle( VectorPropertyTrackCreateEditorHandle );
		SequencerModule.UnRegisterTrackEditor_Handle( VisibilityPropertyTrackCreateEditorHandle );

		// unregister specialty track editors
		SequencerModule.UnRegisterTrackEditor_Handle( AnimationTrackCreateEditorHandle );
		SequencerModule.UnRegisterTrackEditor_Handle( AttachTrackCreateEditorHandle );
		SequencerModule.UnRegisterTrackEditor_Handle( AudioTrackCreateEditorHandle );
		SequencerModule.UnRegisterTrackEditor_Handle( EventTrackCreateEditorHandle );
		SequencerModule.UnRegisterTrackEditor_Handle( ParticleTrackCreateEditorHandle );
		SequencerModule.UnRegisterTrackEditor_Handle( PathTrackCreateEditorHandle );
		SequencerModule.UnRegisterTrackEditor_Handle( ShotTrackCreateEditorHandle );
		SequencerModule.UnRegisterTrackEditor_Handle( SlomoTrackCreateEditorHandle );
		SequencerModule.UnRegisterTrackEditor_Handle( SubMovieSceneTrackCreateEditorHandle );
		SequencerModule.UnRegisterTrackEditor_Handle( TransformTrackCreateEditorHandle );
		SequencerModule.UnRegisterTrackEditor_Handle( ComponentMaterialTrackCreatEditorHandle );
		SequencerModule.UnRegisterTrackEditor_Handle( FadeTrackCreatEditorHandle );
	}

private:

	/** Registered delegate handles */
	FDelegateHandle BoolPropertyTrackCreateEditorHandle;
	FDelegateHandle BytePropertyTrackCreateEditorHandle;
	FDelegateHandle ColorPropertyTrackCreateEditorHandle;
	FDelegateHandle FloatPropertyTrackCreateEditorHandle;
	FDelegateHandle VectorPropertyTrackCreateEditorHandle;
	FDelegateHandle VisibilityPropertyTrackCreateEditorHandle;

	FDelegateHandle AnimationTrackCreateEditorHandle;
	FDelegateHandle AttachTrackCreateEditorHandle;
	FDelegateHandle AudioTrackCreateEditorHandle;
	FDelegateHandle EventTrackCreateEditorHandle;
	FDelegateHandle ParticleTrackCreateEditorHandle;
	FDelegateHandle PathTrackCreateEditorHandle;
	FDelegateHandle ShotTrackCreateEditorHandle;
	FDelegateHandle SlomoTrackCreateEditorHandle;
	FDelegateHandle SubMovieSceneTrackCreateEditorHandle;
	FDelegateHandle TransformTrackCreateEditorHandle;
	FDelegateHandle ComponentMaterialTrackCreatEditorHandle;
	FDelegateHandle FadeTrackCreatEditorHandle;
};


IMPLEMENT_MODULE( FMovieSceneToolsModule, MovieSceneTools );
