FVertexFactoryShaderParameters* FFlexGPUVertexFactory::ConstructShaderParameters(EShaderFrequency ShaderFrequency)
{
	return ShaderFrequency == SF_Vertex ? new FFlexMeshVertexFactoryShaderParameters() : NULL;
}

FFlexGPUVertexFactory::FFlexGPUVertexFactory(const FLocalVertexFactory& Base, const FVertexBuffer* ClusterWeightsVertexBuffer, const FVertexBuffer* ClusterIndicesVertexBuffer)
{
	// set our streams
	FlexData.ClusterWeights = FVertexStreamComponent(ClusterWeightsVertexBuffer, 0, sizeof(float) * 4, VET_Float4);
	FlexData.ClusterIndices = FVertexStreamComponent(ClusterIndicesVertexBuffer, 0, sizeof(int16) * 4, VET_Short4);

	// have to first initialize our RHI and then recreate it from the static mesh
	BeginInitResource(this);

	// copy vertex factory from LOD0 of staticmesh
	Copy(Base);
}

FFlexGPUVertexFactory::~FFlexGPUVertexFactory()
{
	ReleaseResource();
}

void FFlexGPUVertexFactory::AddVertexElements(DataType& InData, FVertexDeclarationElementList& Elements)
{
	FLocalVertexFactory::AddVertexElements(Data, Elements);

	// Add Flex elements
	Elements.Add(AccessStreamComponent(FlexData.ClusterIndices, 8));
	Elements.Add(AccessStreamComponent(FlexData.ClusterWeights, 9));
	// And Flex elements
}

void FFlexGPUVertexFactory::AddVertexPositionElements(DataType& Data, FVertexDeclarationElementList& Elements)
{
	FLocalVertexFactory::AddVertexElements(Data, Elements);

	// Add Flex elements
	Elements.Add(AccessStreamComponent(FlexData.ClusterIndices, 8));
	Elements.Add(AccessStreamComponent(FlexData.ClusterWeights, 9));
	// And Flex elements
}

void FFlexGPUVertexFactory::InitDynamicRHI()
{
	if (MaxClusters > 0)
	{
		ClusterTranslations.Initialize(sizeof(FVector4), MaxClusters, PF_A32B32G32R32F, BUF_AnyDynamic);
		ClusterRotations.Initialize(sizeof(FVector4), MaxClusters, PF_A32B32G32R32F, BUF_AnyDynamic);
	}
}

void FFlexGPUVertexFactory::ReleaseDynamicRHI()
{
	if (ClusterTranslations.NumBytes > 0)
	{
		ClusterTranslations.Release();
		ClusterRotations.Release();
	}
}

void FFlexGPUVertexFactory::AllocateFor(int32 InMaxClusters)
{
	if (InMaxClusters > MaxClusters)
	{
		MaxClusters = InMaxClusters;

		if (!IsInitialized())
		{
			InitResource();
		}
		else
		{
			UpdateRHI();
		}
	}
}

void FFlexGPUVertexFactory::SkinCloth(const FVector4* SimulatedPositions, const FVector* SimulatedNormals, const int* VertexToParticleMap)
{
	// todo: implement
	check(0);
}

// for GPU skinning this method just uploads the necessary data to the skinning buffers
void FFlexGPUVertexFactory::SkinSoft(const FPositionVertexBuffer& Positions, const FStaticMeshVertexBuffer& Vertices, const FFlexShapeTransform* Transforms, const FVector* RestPoses, const int16* ClusterIndices, const float* ClusterWeights, int NumClusters)
{
	SCOPE_CYCLE_COUNTER(STAT_Flex_RenderMeshTime);

	AllocateFor(NumClusters);

	if (NumClusters)
	{
		// remove rest pose translation now, rest pose rotation is always the identity so we can send those directly (below)
		FVector4* TranslationData = (FVector4*)RHILockVertexBuffer(ClusterTranslations.Buffer, 0, NumClusters*sizeof(FVector4), RLM_WriteOnly);

		for (int i = 0; i < NumClusters; ++i)
			TranslationData[i] = FVector4(Transforms[i].Translation - Transforms[i].Rotation*RestPoses[i], 0.0f);

		RHIUnlockVertexBuffer(ClusterTranslations.Buffer);

		// rotations
		FQuat* RotationData = (FQuat*)RHILockVertexBuffer(ClusterRotations.Buffer, 0, NumClusters*sizeof(FQuat), RLM_WriteOnly);

		for (int i = 0; i < NumClusters; ++i)
			RotationData[i] = Transforms[i].Rotation;

		RHIUnlockVertexBuffer(ClusterRotations.Buffer);
	}
}



IMPLEMENT_VERTEX_FACTORY_TYPE(FFlexGPUVertexFactory, "LocalVertexFactory", true, true, true, true, true);