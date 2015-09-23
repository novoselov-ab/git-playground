#include "FlexPCH.h"

#include "DynamicMeshBuilder.h"





static int32 CalcNumSegmentsNeeded(float Length, float Radius)
{
	const float ParticleOverlap = 1.6f; // causes the particles to overlap by 60% of their radius 
	const float SafeRadius = FMath::Max(Radius, 0.01f);
	int32 NumSegments = static_cast<int32>(ParticleOverlap * Length / SafeRadius);
	NumSegments = FMath::Min(NumSegments, 2000);
	NumSegments = FMath::Max(NumSegments, 1);
	return NumSegments;
}

//////////////////////////////////////////////////////////////////////////

UFlexRopeComponent::UFlexRopeComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	bTickInEditor = true;
	bAutoActivate = true;

	Length = 100.f;
	Width = 10.f;
	NumSegments = 10;
	AutoComputeSegments = true;
	if (ContainerTemplate)
	{
		NumSegments = CalcNumSegmentsNeeded(Length, ContainerTemplate->Radius);
	}

	NumSides = 4;
	EndLocation = FVector(100.0f, 0.0f, 0.0f);
	AttachToRigids = true;	
	StretchStiffness = 1.0f;
	BendStiffness = 0.5f;
	TileMaterial = 1.f;

	SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);

	Asset = NULL;
}

void UFlexRopeComponent::UpdateSceneProxy(FFlexRopeSceneProxy* Proxy)
{
	// Allocate FlexRope dynamic data
	FFlexRopeDynamicData* DynamicData = new FFlexRopeDynamicData;

	// Transform current positions from particles into component-space array
	int32 NumPoints = NumSegments + 1;
	DynamicData->FlexRopePoints.AddUninitialized(NumPoints);
	for (int32 PointIdx = 0; PointIdx<NumPoints; PointIdx++)
	{
		DynamicData->FlexRopePoints[PointIdx] = ComponentToWorld.InverseTransformPosition(FVector(Particles[PointIdx]));
	}

	// Enqueue command to send to render thread
	ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
		FSendFlexRopeDynamicData,
		FFlexRopeSceneProxy*, Proxy, Proxy,
		FFlexRopeDynamicData*, DynamicData, DynamicData,
		{
			Proxy->SetDynamicData_RenderThread(DynamicData);
		});
}

FPrimitiveSceneProxy* UFlexRopeComponent::CreateSceneProxy()
{
	FFlexRopeSceneProxy* Proxy = new FFlexRopeSceneProxy(this);
	UpdateSceneProxy(Proxy);

	return Proxy;
}

int32 UFlexRopeComponent::GetNumMaterials() const
{
	return 1;
}

#if WITH_EDITOR
void UFlexRopeComponent::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (AutoComputeSegments && (PropertyName == FName(TEXT("ContainerTemplate")) || PropertyName == FName(TEXT("AutoComputeSegments")) || PropertyName == FName(TEXT("Length"))))
	{
		if (ContainerTemplate)
		{
			NumSegments = CalcNumSegmentsNeeded(Length, ContainerTemplate->Radius);
		}
	}
}
#endif

void UFlexRopeComponent::OnRegister()
{
	Super::OnRegister();
#if WITH_FLEX

	// create initial geometry
	CreateRopeGeometry();

	// set up physics
	FPhysScene* PhysScene = GetWorld()->GetPhysicsScene();

	if (ContainerTemplate && PhysScene && (!GIsEditor || GIsPlayInEditorWorld) && !AssetInstance)
	{
		FFlexContainerInstance* Container = PhysScene->GetFlexContainer(ContainerTemplate);
		if (Container)
		{
			ContainerInstance = Container;
			ContainerInstance->Register(this);

			Asset = new FlexExtAsset();
			FMemory::Memset(Asset, 0, sizeof(FlexExtAsset));

			// particles
			Asset->mNumParticles = Particles.Num();

			// particles
			if (Asset->mNumParticles)
				Asset->mParticles = (float*)&Particles[0];

			// distance constraints
			Asset->mNumSprings = SpringCoefficients.Num();
			if (Asset->mNumSprings)
			{
				Asset->mSpringIndices = (int*)&SpringIndices[0];
				Asset->mSpringCoefficients = (float*)&SpringCoefficients[0];
				Asset->mSpringRestLengths = (float*)&SpringLengths[0];
			}

			AssetInstance = Container->CreateInstance(Asset, FMatrix::Identity, FVector(0.0f), Container->GetPhase(Phase));
		}
	}
#endif //WITH_FLEX
}

void UFlexRopeComponent::OnUnregister()
{
	Super::OnUnregister();

#if WITH_FLEX

	if (ContainerInstance)
	{
		ContainerInstance->DestroyInstance(AssetInstance);
		AssetInstance = NULL;

		delete Asset;
	}

	if (ContainerInstance)
	{
		ContainerInstance->Unregister(this);
		ContainerInstance = NULL;
	}

#endif // WITH_FLEX

}

void UFlexRopeComponent::GetEndPositions(FVector& OutStartPosition, FVector& OutEndPosition)
{
	OutStartPosition = GetComponentLocation();
	OutEndPosition = ComponentToWorld.TransformPosition(EndLocation);
}

void UFlexRopeComponent::CreateRopeGeometry()
{
	// create rope geometry
	Particles.Reset();
	SpringIndices.Reset();
	SpringLengths.Reset();
	SpringCoefficients.Reset();

	FVector FlexRopeStart, FlexRopeEnd;
	GetEndPositions(FlexRopeStart, FlexRopeEnd);

	const int32 NumParticles = NumSegments + 1;

	const FVector Delta = FlexRopeEnd - FlexRopeStart;
	const float RestDistance = Length / NumSegments;

	for (int32 ParticleIdx = 0; ParticleIdx<NumParticles; ParticleIdx++)
	{
		const float Alpha = (float)ParticleIdx / (float)NumSegments;
		const FVector InitialPosition = FlexRopeStart + (Alpha * Delta);

		Particles.Add(FVector4(InitialPosition, 1.0f));

		// create springs between particles
		if (ParticleIdx > 0 && StretchStiffness > 0.0f)
		{
			const int P0 = ParticleIdx - 1;
			const int P1 = ParticleIdx;

			SpringIndices.Add(P0);
			SpringIndices.Add(P1);

			SpringLengths.Add(RestDistance);
			SpringCoefficients.Add(StretchStiffness);
		}

		// create bending springs (connect over three particles)
		if (ParticleIdx > 1 && BendStiffness > 0.0f)
		{
			const int P0 = ParticleIdx - 2;
			const int P1 = ParticleIdx;

			SpringIndices.Add(P0);
			SpringIndices.Add(P1);

			SpringLengths.Add(2.0f * RestDistance);
			SpringCoefficients.Add(BendStiffness);
		}
	}
}

void UFlexRopeComponent::Synchronize()
{
#if WITH_FLEX
	if (ContainerInstance && AssetInstance)
	{
		// if attach requested then generate attachment points for overlapping shapes
		if (AttachToRigids)
		{
			// clear out any previous attachments
			Attachments.SetNum(0);

			for (int ParticleIndex = 0; ParticleIndex < AssetInstance->mNumParticles; ++ParticleIndex)
			{
				FVector4 ParticlePos = Particles[ParticleIndex];

				// perform a point check (small sphere)
				FCollisionShape Shape;
				Shape.SetSphere(0.001f);

				// gather overlapping primitives
				TArray<FOverlapResult> Overlaps;
				GetWorld()->OverlapMultiByObjectType(Overlaps, ParticlePos, FQuat::Identity, FCollisionObjectQueryParams(FCollisionObjectQueryParams::InitType::AllObjects), Shape, FCollisionQueryParams(false));

				// pick first non-flex actor, that has a body and is not a trigger
				UPrimitiveComponent* PrimComp = NULL;
				for (int32 OverlapIdx = 0; OverlapIdx<Overlaps.Num() && !PrimComp; ++OverlapIdx)
				{
					FOverlapResult const& O = Overlaps[OverlapIdx];

					if (!O.Component.IsValid() || O.Component.Get()->IsA(UFlexComponent::StaticClass()))
						continue;

					UPrimitiveComponent* TmpPrimComp = O.Component.Get();

					if (TmpPrimComp->GetBodyInstance() == NULL)
						continue;

					ECollisionResponse Response = TmpPrimComp->GetCollisionResponseToChannel(ContainerInstance->Template->ObjectType);
					if (Response == ECollisionResponse::ECR_Ignore)
						continue;

					PrimComp = TmpPrimComp;
				}

				if (PrimComp)
				{
					FBodyInstance* Body = PrimComp->GetBodyInstance();

					if (!Body)
						continue;

					// calculate local space position of particle in component
					FTransform LocalToWorld = PrimComp->GetComponentToWorld();
					FVector LocalPos = LocalToWorld.InverseTransformPosition(ParticlePos);

					FlexParticleAttachment Attachment;
					Attachment.Primitive = PrimComp;
					Attachment.ParticleIndex = ParticleIndex;
					Attachment.OldMass = ParticlePos.W;
					Attachment.LocalPos = LocalPos;
					Attachment.ShapeIndex = 0;	// don't currently support shape indices

					Attachments.Add(Attachment);
				}
			}

			// reset attach flag
			AttachToRigids = false;
		}

		// process attachments
		for (int AttachmentIndex = 0; AttachmentIndex < Attachments.Num();)
		{
			const FlexParticleAttachment& Attachment = Attachments[AttachmentIndex];
			const UPrimitiveComponent* PrimComp = Attachment.Primitive.Get();

			// index into the simulation data, we need to modify the container's copy
			// of the data so that the new positions get sent back to the sim
			const int ParticleIndex = AssetInstance->mParticleIndices[Attachment.ParticleIndex];

			if (PrimComp)
			{
				// calculate world position of attached particle, and zero mass
				const FTransform PrimTransform = PrimComp->GetComponentToWorld();
				const FVector& AttachedPos = PrimTransform.TransformPosition(Attachment.LocalPos);

				ContainerInstance->Particles[ParticleIndex] = FVector4(AttachedPos, 0.0f);
				ContainerInstance->Velocities[ParticleIndex] = FVector(0.0f);

				++AttachmentIndex;
			}
			else // process detachments
			{
				ContainerInstance->Particles[ParticleIndex].W = Attachment.OldMass;
				ContainerInstance->Velocities[ParticleIndex] = FVector(0.0f);

				Attachments.RemoveAt(AttachmentIndex);
			}
		}

		// Copy simulation data back to local array
		if (ContainerInstance && AssetInstance)
		{
			for (int i = 0; i < Particles.Num(); ++i)
			{
				Particles[i] = ContainerInstance->Particles[AssetInstance->mParticleIndices[i]];
			}
		}
	}
#endif //WITH_FLEX
}

void UFlexRopeComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!AssetInstance)
	{
		// if we're not actively being simulated then just update the rope geometry each 
		// frame, this ensures the editor view is updated when modifying parameters
		CreateRopeGeometry();
	}

	// Need to send new data to render thread
	MarkRenderDynamicDataDirty();

	// Call this because bounds have changed
	UpdateComponentToWorld();
}

void UFlexRopeComponent::SendRenderDynamicData_Concurrent()
{
	if (SceneProxy)
	{
		UpdateSceneProxy((FFlexRopeSceneProxy*)SceneProxy);
	}
}

FBoxSphereBounds UFlexRopeComponent::CalcBounds(const FTransform & LocalToWorld) const
{
	// Calculate bounding box of FlexRope points
	FBox RopeBox(0);
	for(int32 ParticleIdx=0; ParticleIdx<Particles.Num(); ParticleIdx++)
	{
		const FVector& Position = FVector(Particles[ParticleIdx]);
		RopeBox += Position;
	}

	// Expand by rope width
	FBoxSphereBounds Bounds = FBoxSphereBounds(RopeBox.ExpandBy(Width));

	// Clamp bounds in case of instability
	const float MaxRadius = 1000000.0f;

	if (Bounds.SphereRadius > MaxRadius)
		Bounds = FBoxSphereBounds(EForceInit::ForceInitToZero);

	return Bounds;
}

