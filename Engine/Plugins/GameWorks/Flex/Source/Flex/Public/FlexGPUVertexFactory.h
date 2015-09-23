#pragma once
#include "FlexVertexFactory.h"

class FFlexGPUVertexFactory : public FFlexVertexFactory
{
	DECLARE_VERTEX_FACTORY_TYPE(FFlexVertexFactory);

public:

	struct FlexDataType
	{
		/** Skinning weights for clusters */
		FVertexStreamComponent ClusterWeights;

		/** Skinning indices for clusters */
		FVertexStreamComponent ClusterIndices;
	};

	/** Should we cache the material's shadertype on this platform with this vertex factory? */
	static bool ShouldCache(EShaderPlatform Platform, const class FMaterial* Material, const class FShaderType* ShaderType)
	{
		return  IsFeatureLevelSupported(Platform, ERHIFeatureLevel::SM4) &&
			(Material->IsUsedWithFlexMeshes() || Material->IsSpecialEngineMaterial()) &&
			FLocalVertexFactory::ShouldCache(Platform, Material, ShaderType);
	}

	/** Modify compile environment to enable flex cluster deformation */
	static void ModifyCompilationEnvironment(EShaderPlatform Platform, const FMaterial* Material, FShaderCompilerEnvironment& OutEnvironment)
	{
		FLocalVertexFactory::ModifyCompilationEnvironment(Platform, Material, OutEnvironment);
		OutEnvironment.SetDefine(TEXT("USE_FLEX_DEFORM"), TEXT("1"));
	}

	static FVertexFactoryShaderParameters* ConstructShaderParameters(EShaderFrequency ShaderFrequency);

public:

	FFlexGPUVertexFactory(const FLocalVertexFactory& Base, const FVertexBuffer* ClusterWeightsVertexBuffer, const FVertexBuffer* ClusterIndicesVertexBuffer);
	virtual ~FFlexGPUVertexFactory();

	virtual void AddVertexElements(DataType& InData, FVertexDeclarationElementList& Elements) override;
	virtual void AddVertexPositionElements(DataType& Data, FVertexDeclarationElementList& Elements) override;

	virtual void InitDynamicRHI();
	virtual void ReleaseDynamicRHI();

	void AllocateFor(int32 InMaxClusters);

	// FFlexVertexFactory methods
	virtual void SkinCloth(const FVector4* SimulatedPositions, const FVector* SimulatedNormals, const int* VertexToParticleMap) override;
	virtual void SkinSoft(const FPositionVertexBuffer& Positions, const FStaticMeshVertexBuffer& Vertices, const FFlexShapeTransform* Transforms, const FVector* RestPoses, const int16* ClusterIndices, const float* ClusterWeights, int NumClusters) override;

	int MaxClusters;

	FReadBuffer ClusterTranslations;
	FReadBuffer ClusterRotations;

protected:

	FlexDataType FlexData;
};