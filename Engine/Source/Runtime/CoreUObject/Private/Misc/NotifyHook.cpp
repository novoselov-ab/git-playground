// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

/*=============================================================================
	UnMisc.cpp: Various core platform-independent functions.
=============================================================================*/

// Core includes.
#include "CoreUObjectPrivate.h"
#include "Misc/NotifyHook.h"

void FNotifyHook::NotifyPreChange( FEditPropertyChain* PropertyAboutToChange )
{
	NotifyPreChange( PropertyAboutToChange != NULL && PropertyAboutToChange->Num() > 0 ? PropertyAboutToChange->GetActiveNode()->GetValue() : NULL );
}

void FNotifyHook::NotifyPostChange( const FPropertyChangedEvent& PropertyChangedEvent, class FEditPropertyChain* PropertyThatChanged )
{
	NotifyPostChange( PropertyChangedEvent, PropertyThatChanged != NULL && PropertyThatChanged->Num() > 0 ? PropertyThatChanged->GetActiveNode()->GetValue() : NULL );
}


