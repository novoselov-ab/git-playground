#include "FlexPCH.h"
#include "PhysicsHooks.h"

AFlexGlobalDataActor::AFlexGlobalDataActor(const FObjectInitializer &ObjectInitializer):
	AActor(ObjectInitializer)
{
	//TODO: Not visible in game!
	//TODO: Proper ticks!
}

AFlexGlobalDataActor::~AFlexGlobalDataActor()
{
	// Wait for GPU work to finish before the FlexContainerMap is allowed to dealloc itself.
	flexWaitFence();

	FPhysicsHooks::get().PhysicsSceneEndOfStartFrameCallbacks.Remove(PostFrameCallbackHandle);
}

TSharedPtr<FFlexContainerInstance> AFlexGlobalDataActor::GetContainerInstance(UFlexContainer *ContainerTypeKey)
{
	TSharedPtr<FFlexContainerInstance> instance = FlexContainerMap.Find(ContainerTypeKey);

	if (!instance.IsValid())
	{
		instance = MakeShareable<FFlexContainerInstance>(new FFlexContainerInstance(ContainerTypeKey));
		FlexContainerMap.Add(ContainerTypeKey, instance);
	}

	return *instance;
}

AFlexGlobalDataActor* AFlexGlobalDataActor::GetGlobalDataActorFromWorld(UWorld *World)
{
	AFlexGlobalDataActor* GlobalDataActor = nullptr;

	// Get the global data actor, if it's in the scene.
	for (TActorIterator<AFlexGlobalDataActor> ActorItr(World); ActorItr; ++ActorItr)
	{
		GlobalDataActor = *ActorItr;
		break;
	}

	// Not there quite yet, add it.
	if (GlobalDataActor == nullptr)
	{
		UE_LOG(LogFlex, Log, TEXT("Spawning AFlexGlobalDataActor into scene."));

		GlobalDataActor = World->SpawnActor<AFlexGlobalDataActor>(AFlexGlobalDataActor::StaticClass());
	}

	return GlobalDataActor;
}

void AFlexGlobalDataActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	PostFrameCallbackHandle = FPhysicsHooks::get().PhysicsSceneEndOfStartFrameCallbacks.Add(std::bind(&AFlexGlobalDataActor::TickFlexScenes, this, std::placeholders::_1), 0);
}

void AFlexGlobalDataActor::TickFlexScenes(float DeltaSeconds)
{
	check(GPhysXSDK);
	check(DeltaSeconds > 0.0f);

	// JDM: Why do we do this? This seems wrong.
	GPhysXSDK->getPvdConnectionManager()->disconnect();

	for (auto It = FlexContainerMap.CreateIterator(); It; ++It)
	{
		// if template has been garbage collected then remove container
		if (!It->Value->Template.IsValid())
		{
			It.RemoveCurrent();
		}
		else
		{
			It->Value->Simulate(DeltaSeconds);
		}
	}

	if (FlexContainerMap.Num())
		flexSetFence();
}
