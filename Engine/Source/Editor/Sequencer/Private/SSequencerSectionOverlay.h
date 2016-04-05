// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "TimeSliderController.h"

/**
 * An overlay that displays global information in the section area
 */
class SSequencerSectionOverlay : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS( SSequencerSectionOverlay )
		: _DisplayTickLines( true )
		, _DisplayScrubPosition( false )
	{}
		SLATE_ATTRIBUTE( bool, DisplayTickLines )
		SLATE_ATTRIBUTE( bool, DisplayScrubPosition )
		SLATE_ATTRIBUTE( FPaintPlaybackRangeArgs, PaintPlaybackRangeArgs )

	SLATE_END_ARGS()

	void Construct( const FArguments& InArgs, TSharedRef<FSequencerTimeSliderController> InTimeSliderController )
	{
		bDisplayScrubPosition = InArgs._DisplayScrubPosition;
		bDisplayTickLines = InArgs._DisplayTickLines;
		PaintPlaybackRangeArgs = InArgs._PaintPlaybackRangeArgs;
		TimeSliderController = InTimeSliderController;
	}

private:
	/** SWidget Interface */
	virtual int32 OnPaint( const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled ) const override;

private:
	/** Controller for manipulating time */
	TSharedPtr<FSequencerTimeSliderController> TimeSliderController;
	/** Whether or not to display the scrub position */
	TAttribute<bool> bDisplayScrubPosition;
	/** Whether or not to display tick lines */
	TAttribute<bool> bDisplayTickLines;
	/** User-supplied options for drawing playback range */
	TAttribute<FPaintPlaybackRangeArgs> PaintPlaybackRangeArgs;

};