#include "FlexEditorPCH.h"

#include "FlexAsset.h"
#include "FlexContainer.h"
#include "FlexAssetSoft.h"
#include "DynamicMeshBuilder.h"

FFlexAssetPreviewSceneProxy::FFlexAssetPreviewSceneProxy(const UFlexAssetPreviewComponent* InComponent) : FPrimitiveSceneProxy(InComponent)
{
	bWillEverBeLit = true;
	bNeedsUnbuiltPreviewLighting = true;

	ViewRelevance.bDrawRelevance = true;
	ViewRelevance.bDynamicRelevance = true;
	ViewRelevance.bNormalTranslucencyRelevance = true;

	UFlexAsset* FlexAsset = InComponent->FlexAsset;
	UFlexContainer* Container = FlexAsset ? FlexAsset->ContainerTemplate : NULL;

	if (FlexAsset && Container)
	{
		//UFlexContainer::Radius represents rest spacing, which corresponds to two spheres of radii Radius/2 touching.
		float Radius = Container->Radius*0.5f;
		float InvMass = (FlexAsset->Mass > 0.0f) ? (1.0f / FlexAsset->Mass) : 0.0f;

		for (int32 ParticleIndex = 0; ParticleIndex < FlexAsset->Particles.Num(); ++ParticleIndex)
		{
			FVector4& Particle = FlexAsset->Particles[ParticleIndex];
			FVector Position(Particle.X, Particle.Y, Particle.Z);
			uint8 MassColVal = uint8(Particle.W * 255.0f / InvMass);
			FColor Color = FColor(MassColVal, 0, 0, 255);

			AddSolidSphere(Position, Radius, Color, 7, 7);
		}
	}

	UFlexAssetSoft* FlexSoftAsset = Cast<UFlexAssetSoft>(FlexAsset);

	if (FlexSoftAsset)
	{
		Lines.Empty();

		// build cluster rings
		for (int i = 0; i < FlexSoftAsset->ShapeCenters.Num(); ++i)
		{
			AddBasis(FlexSoftAsset->ShapeCenters[i], FlexSoftAsset->ClusterRadius);
		}

		// build links
		for (int i = 0; i < FlexSoftAsset->SpringCoefficients.Num(); ++i)
		{
			int Particle0 = FlexSoftAsset->SpringIndices[i * 2 + 0];
			int Particle1 = FlexSoftAsset->SpringIndices[i * 2 + 1];

			AddLine(FlexSoftAsset->Particles[Particle0], FlexSoftAsset->Particles[Particle1], FColor::Cyan);
		}
	}
}

void FFlexAssetPreviewSceneProxy::AddSolidSphere(FVector Position, float Radius, FColor Color, int32 NumSides, int32 NumRings)
{
	// The first/last arc are on top of each other.
	int32 NumVerts = (NumSides + 1) * (NumRings + 1);
	FDynamicMeshVertex* Verts = (FDynamicMeshVertex*)FMemory::Malloc(NumVerts * sizeof(FDynamicMeshVertex));

	// Calculate verts for one arc
	FDynamicMeshVertex* ArcVerts = (FDynamicMeshVertex*)FMemory::Malloc((NumRings + 1) * sizeof(FDynamicMeshVertex));

	for (int32 i = 0; i < NumRings + 1; i++)
	{
		FDynamicMeshVertex* ArcVert = &ArcVerts[i];

		float angle = ((float)i / NumRings) * PI;

		// Note- unit sphere, so position always has mag of one. We can just use it for normal!			
		ArcVert->Position.X = 0.0f;
		ArcVert->Position.Y = FMath::Sin(angle);
		ArcVert->Position.Z = FMath::Cos(angle);

		ArcVert->SetTangents(
			FVector(1, 0, 0),
			FVector(0.0f, -ArcVert->Position.Z, ArcVert->Position.Y),
			ArcVert->Position
			);

		ArcVert->TextureCoordinate.X = 0.0f;
		ArcVert->TextureCoordinate.Y = ((float)i / NumRings);
		ArcVert->Color = Color;
	}

	// Then rotate this arc NumSides+1 times.
	for (int32 s = 0; s < NumSides + 1; s++)
	{
		FRotator ArcRotator(0, 360.f * (float)s / NumSides, 0);
		FRotationMatrix ArcRot(ArcRotator);
		float XTexCoord = ((float)s / NumSides);

		for (int32 v = 0; v < NumRings + 1; v++)
		{
			int32 VIx = (NumRings + 1)*s + v;

			Verts[VIx].Position = ArcRot.TransformPosition(ArcVerts[v].Position);

			Verts[VIx].SetTangents(
				ArcRot.TransformVector(ArcVerts[v].TangentX),
				ArcRot.TransformVector(ArcVerts[v].GetTangentY()),
				ArcRot.TransformVector(ArcVerts[v].TangentZ)
				);

			Verts[VIx].TextureCoordinate.X = XTexCoord;
			Verts[VIx].TextureCoordinate.Y = ArcVerts[v].TextureCoordinate.Y;
			Verts[VIx].Color = Color;
		}
	}

	// Add all of the vertices we generated to the mesh builder.
	int32 VertexOffset = Vertices.Num();
	Vertices.AddUninitialized(NumVerts);
	for (int32 VertIdx = 0; VertIdx < NumVerts; VertIdx++)
	{
		FDynamicMeshVertex Vertex = Verts[VertIdx];
		Vertex.Position = Vertex.Position*Radius + Position;
		Vertices[VertexOffset + VertIdx] = Vertex;
	}

	// Add all of the triangles we generated to the mesh builder.
	int32 TriangleOffset = Triangles.Num();
	Triangles.AddUninitialized(NumSides*NumRings * 2 * 3);
	int32 Index = 0;
	for (int32 s = 0; s < NumSides; s++)
	{
		int32 a0start = (s + 0) * (NumRings + 1);
		int32 a1start = (s + 1) * (NumRings + 1);

		for (int32 r = 0; r < NumRings; r++)
		{
			Triangles[TriangleOffset + Index++] = a0start + r + 0 + VertexOffset;
			Triangles[TriangleOffset + Index++] = a1start + r + 0 + VertexOffset;
			Triangles[TriangleOffset + Index++] = a0start + r + 1 + VertexOffset;

			Triangles[TriangleOffset + Index++] = a1start + r + 0 + VertexOffset;
			Triangles[TriangleOffset + Index++] = a1start + r + 1 + VertexOffset;
			Triangles[TriangleOffset + Index++] = a0start + r + 1 + VertexOffset;
		}
	}

	// Free our local copy of verts and arc verts
	FMemory::Free(Verts);
	FMemory::Free(ArcVerts);
}

void FFlexAssetPreviewSceneProxy::AddLine(FVector Start, FVector End, FColor Color)
{
	Line L;
	L.Start = Start;
	L.End = End;
	L.Color = Color;

	Lines.Add(L);
}

void FFlexAssetPreviewSceneProxy::AddBasis(FVector Position, float Length)
{
	AddLine(Position, Position + FVector(Length, 0.0f, 0.0f), FColor::Red);
	AddLine(Position, Position + FVector(0.0f, Length, 0.0f), FColor::Green);
	AddLine(Position, Position + FVector(0.0f, 0.0f, Length), FColor::Blue);
}

void FFlexAssetPreviewSceneProxy::GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, class FMeshElementCollector& Collector) const
{
	for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
	{
		if (VisibilityMap & (1 << ViewIndex))
		{
			const FSceneView* View = Views[ViewIndex];
			FPrimitiveDrawInterface* PDI = Collector.GetPDI(ViewIndex);

			FDynamicMeshBuilder MeshBuilder;
			MeshBuilder.AddVertices(Vertices);
			MeshBuilder.AddTriangles(Triangles);

			UMaterial* Material = GEngine->ShadedLevelColorationLitMaterial;
			if (View->Family->EngineShowFlags.VertexColors)
			{
				Material = GEngine->VertexColorViewModeMaterial_ColorOnly;
			}
			else if (View->Family->EngineShowFlags.Wireframe)
			{
				Material = GEngine->WireframeMaterial;
			}

			MeshBuilder.GetMesh(FMatrix::Identity, Material->GetRenderProxy(false), SDPG_World, false, false, ViewIndex, Collector);

			// draw clusters
			for (int32 i = 0; i < Lines.Num(); i++)
				PDI->DrawLine(Lines[i].Start, Lines[i].End, Lines[i].Color, SDPG_Foreground, 0.0f);
		}
	}
}

FPrimitiveViewRelevance FFlexAssetPreviewSceneProxy::GetViewRelevance(const FSceneView* View)
{
	return ViewRelevance;
}

uint32 FFlexAssetPreviewSceneProxy::GetMemoryFootprint(void) const
{
	return(sizeof(*this) + GetAllocatedSize());
}

uint32 FFlexAssetPreviewSceneProxy::GetAllocatedSize(void) const
{
	//size not correct
	return(FPrimitiveSceneProxy::GetAllocatedSize() + Vertices.GetAllocatedSize() + Triangles.GetAllocatedSize());
}

