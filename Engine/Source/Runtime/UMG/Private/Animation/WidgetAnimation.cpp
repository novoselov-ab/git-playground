// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "UMGPrivatePCH.h"
#include "MovieScene.h"
#include "WidgetAnimation.h"
#include "WidgetTree.h"


#define LOCTEXT_NAMESPACE "UWidgetAnimation"


/* UWidgetAnimation structors
 *****************************************************************************/

UWidgetAnimation::UWidgetAnimation(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, MovieScene(nullptr)
{ }


/* UWidgetAnimation interface
 *****************************************************************************/

#if WITH_EDITOR

UWidgetAnimation* UWidgetAnimation::GetNullAnimation()
{
	static UWidgetAnimation* NullAnimation = nullptr;

	if (!NullAnimation)
	{
		NullAnimation = NewObject<UWidgetAnimation>(GetTransientPackage(), NAME_None, RF_RootSet);
		NullAnimation->MovieScene = NewObject<UMovieScene>(NullAnimation, FName("No Animation"), RF_RootSet);
	}

	return NullAnimation;
}

#endif


float UWidgetAnimation::GetStartTime() const
{
	return MovieScene->GetTimeRange().GetLowerBoundValue();
}


float UWidgetAnimation::GetEndTime() const
{
	return MovieScene->GetTimeRange().GetUpperBoundValue();
}


void UWidgetAnimation::Initialize(UUserWidget* InPreviewWidget)
{
	PreviewWidget = InPreviewWidget;

	// clear object maps
	PreviewObjectToIds.Empty();
	IdToPreviewObjects.Empty();
	IdToSlotContentPreviewObjects.Empty();
	SlotContentPreviewObjectToIds.Empty();

	if (PreviewWidget == nullptr)
	{
		return;
	}

	// populate object maps
	UWidgetTree* WidgetTree = PreviewWidget->WidgetTree;

	for (const FWidgetAnimationBinding& Binding : AnimationBindings)
	{
		UObject* FoundObject = Binding.FindRuntimeObject(*WidgetTree);

		if (FoundObject == nullptr)
		{
			continue;
		}

		UPanelSlot* FoundSlot = Cast<UPanelSlot>(FoundObject);

		if (FoundSlot == nullptr)
		{
			IdToPreviewObjects.Add(Binding.AnimationGuid, FoundObject);
			PreviewObjectToIds.Add(FoundObject, Binding.AnimationGuid);
		}
		else
		{
			IdToSlotContentPreviewObjects.Add(Binding.AnimationGuid, FoundSlot->Content);
			SlotContentPreviewObjectToIds.Add(FoundSlot->Content, Binding.AnimationGuid);
		}
	}
}


/* UMovieSceneAnimation overrides
 *****************************************************************************/

bool UWidgetAnimation::AllowsSpawnableObjects() const
{
	return false;
}


void UWidgetAnimation::BindPossessableObject(const FGuid& ObjectId, UObject& PossessedObject)
{
	UPanelSlot* PossessedSlot = Cast<UPanelSlot>(&PossessedObject);

	if ((PossessedSlot != nullptr) && (PossessedSlot->Content != nullptr))
	{
		SlotContentPreviewObjectToIds.Add(PossessedSlot->Content, ObjectId);
		IdToSlotContentPreviewObjects.Add(ObjectId, PossessedSlot->Content);

		// Save the name of the widget containing the slots. This is the object
		// to look up that contains the slot itself (the thing we are animating).
		FWidgetAnimationBinding NewBinding;
		{
			NewBinding.AnimationGuid = ObjectId;
			NewBinding.SlotWidgetName = PossessedSlot->GetFName();
			NewBinding.WidgetName = PossessedSlot->Content->GetFName();
		}

		AnimationBindings.Add(NewBinding);
	}
	else if (PossessedSlot == nullptr)
	{
		PreviewObjectToIds.Add(&PossessedObject, ObjectId);
		IdToPreviewObjects.Add(ObjectId, &PossessedObject);

		FWidgetAnimationBinding NewBinding;
		{
			NewBinding.AnimationGuid = ObjectId;
			NewBinding.WidgetName = PossessedObject.GetFName();
		}

		AnimationBindings.Add(NewBinding);
	}
}


bool UWidgetAnimation::CanPossessObject(UObject& Object) const
{
	UPanelSlot* Slot = Cast<UPanelSlot>(&Object);

	if ((Slot != nullptr) && (Slot->Content == nullptr))
	{
		// can't possess empty slots.
		return false;
	}

	return (Object.IsA<UVisual>() && Object.IsIn(PreviewWidget.Get()));
}


FGuid UWidgetAnimation::FindObjectId(UObject& Object) const
{
	UPanelSlot* Slot = Cast<UPanelSlot>(&Object);

	if (Slot != nullptr)
	{
		// slot guids are tracked by their content.
		return SlotContentPreviewObjectToIds.FindRef(Slot->Content);
	}

	return PreviewObjectToIds.FindRef(&Object);
}


UObject* UWidgetAnimation::FindObject(const FGuid& ObjectId) const
{
	TWeakObjectPtr<UObject> PreviewObject = IdToPreviewObjects.FindRef(ObjectId);

	if (PreviewObject.IsValid())
	{
		return PreviewObject.Get();
	}

	TWeakObjectPtr<UObject> SlotContentPreviewObject = IdToSlotContentPreviewObjects.FindRef(ObjectId);

	if (SlotContentPreviewObject.IsValid())
	{
		UWidget* Widget = Cast<UWidget>(SlotContentPreviewObject.Get());
		if ( Widget != nullptr )
		{
			return Widget->Slot;
		}
	}
	
	return nullptr;
}


UMovieScene* UWidgetAnimation::GetMovieScene() const
{
	return MovieScene;
}


UObject* UWidgetAnimation::GetParentObject(UObject* Object) const
{
	UPanelSlot* Slot = Cast<UPanelSlot>(Object);

	if (Slot != nullptr)
	{
		// The slot is actually the child of the panel widget in the hierarchy,
		// but we want it to show up as a sub-object of the widget it contains
		// in the timeline so we return the content's GUID.
		return Slot->Content;
	}

	return nullptr;
}


#if WITH_EDITOR
bool UWidgetAnimation::TryGetObjectDisplayName(const FGuid& ObjectId, FText& OutDisplayName) const
{
	// TODO: This gets called every frame for every bound object and could
	// be a potential performance issue for a really complicated animation.

	TWeakObjectPtr<UObject> PreviewObject = IdToPreviewObjects.FindRef(ObjectId);

	if (PreviewObject.IsValid())
	{
		OutDisplayName = FText::FromString(PreviewObject.Get()->GetName());
		return true;
	}

	TWeakObjectPtr<UObject> SlotContentPreviewObject = IdToSlotContentPreviewObjects.FindRef(ObjectId);

	if (SlotContentPreviewObject.IsValid())
	{
		UWidget* SlotContent = Cast<UWidget>(SlotContentPreviewObject.Get());
		FText PanelName = SlotContent->Slot != nullptr && SlotContent->Slot->Parent != nullptr
			? FText::FromString(SlotContent->Slot->Parent->GetName())
			: LOCTEXT("InvalidPanel", "Invalid Panel");
		FText ContentName = FText::FromString(SlotContent->GetName());
		if ( PreviewObjectToIds.Contains( SlotContent ) )
		{
			OutDisplayName = FText::Format( LOCTEXT( "SlotObjectWithParent", "{0} Slot" ), PanelName );
		}
		else
		{
			OutDisplayName = FText::Format(LOCTEXT("SlotObject", "{0} ({1} Slot)"), ContentName, PanelName);
		}

		return true;
	}

	return false;
}
#endif


void UWidgetAnimation::UnbindPossessableObjects(const FGuid& ObjectId)
{
	// unbind widgets
	TArray<TWeakObjectPtr<UObject>> PreviewObjects;
	{
		IdToPreviewObjects.MultiFind(ObjectId, PreviewObjects);

		for (TWeakObjectPtr<UObject>& PreviewObject : PreviewObjects)
		{
			PreviewObjectToIds.Remove(PreviewObject);
		}

		IdToPreviewObjects.Remove(ObjectId);
	}

	// unbind slots
	TArray<TWeakObjectPtr<UObject>> SlotContentPreviewObjects;
	{
		IdToSlotContentPreviewObjects.MultiFind(ObjectId, SlotContentPreviewObjects);

		for (TWeakObjectPtr<UObject>& SlotContentPreviewObject : SlotContentPreviewObjects)
		{
			SlotContentPreviewObjectToIds.Remove(SlotContentPreviewObject);
		}

		IdToSlotContentPreviewObjects.Remove(ObjectId);
	}

	// mark dirty
	Modify();

	// remove animation bindings
	AnimationBindings.RemoveAll([&](const FWidgetAnimationBinding& Binding) {
		return Binding.AnimationGuid == ObjectId;
	});
}


#undef LOCTEXT_NAMESPACE
