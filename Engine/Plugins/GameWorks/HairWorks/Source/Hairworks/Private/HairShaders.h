#pragma once

#include "GlobalShader.h"
#include "ShaderParameters.h"
#include "RHICommandList.h"
#include "SceneView.h"
#include "RHIResources.h"
//#include "D3D11RHIPrivate.h"	// Hack

#define _CPP

#include "AllowWindowsPlatformTypes.h"
#pragma warning(push)
#pragma warning(disable: 4191)	// For DLL function pointer conversion
#include "GFSDK_HairWorks.h"
#pragma warning(pop)
#include "HideWindowsPlatformTypes.h"

#include "GFSDK_Types.h"

//#include "../../../../Shaders/GFSDK_HairWorks_ShaderCommon.usf"

// Pixel shaders
class FHairWorksPs : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FHairWorksPs, Global);
public:
	FHairWorksPs();

	FHairWorksPs(const ShaderMetaType::CompiledShaderInitializerType& Initializer);

	virtual bool Serialize(FArchive& Ar);

	void SetParameters(FRHICommandListImmediate& RHICmdList, const FSceneView& View, const GFSDK_Hair_ConstantBuffer& HairConstBuffer, const TArray<FTexture2DRHIRef>& HairTextures);//const FTextureRHIParamRef LightCache[3], const FVector LightCatcheAlloc[4]

	static bool ShouldCache(EShaderPlatform Platform);

	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment);

protected:
	FShaderParameter HairConstantBuffer;

	FShaderResourceParameter TextureSampler;

	FShaderResourceParameter RootColorTexture;;
	FShaderResourceParameter TipColorTexture;
	FShaderResourceParameter SpecularColorTexture;

	// To suppress warnings.
	FShaderResourceParameter	GFSDK_HAIR_RESOURCE_FACE_HAIR_INDICES;
	FShaderResourceParameter	GFSDK_HAIR_RESOURCE_TANGENTS;
	FShaderResourceParameter	GFSDK_HAIR_RESOURCE_NORMALS;
};


// class FHairWorksSimplePs : public FGlobalShader
// {
// 	DECLARE_SHADER_TYPE(FHairWorksSimplePs, Global);
// 
// 	FHairWorksSimplePs();
// 
// 	FHairWorksSimplePs(const ShaderMetaType::CompiledShaderInitializerType& Initializer);
// 
// 	static bool ShouldCache(EShaderPlatform Platform);
// 
// 	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment);
// };
// 
// 
// class FHairWorksShadowDepthPs : public FGlobalShader
// {
// 	DECLARE_SHADER_TYPE(FHairWorksShadowDepthPs, Global);
// 
// 	FHairWorksShadowDepthPs();
// 
// 	FHairWorksShadowDepthPs(const ShaderMetaType::CompiledShaderInitializerType& Initializer);
// 
// 	bool Serialize(FArchive& Ar) override;
// 
// 	static bool ShouldCache(EShaderPlatform Platform);
// 
// 	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment);
// 
// 	FShaderParameter ShadowParams;
// };
// 
// 
// class FHairWorksVelocityPs : public FGlobalShader
// {
// 	DECLARE_SHADER_TYPE(FHairWorksVelocityPs, Global);
// 
// 	FHairWorksVelocityPs();
// 
// 	FHairWorksVelocityPs(const ShaderMetaType::CompiledShaderInitializerType& Initializer);
// 
// 	bool Serialize(FArchive& Ar) override;
// 
// 	static bool ShouldCache(EShaderPlatform Platform);
// 
// 	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment);
// 
// 	FShaderParameter HairConstantBuffer;
// 
// 	FShaderResourceParameter	GFSDK_HAIR_RESOURCE_FACE_HAIR_INDICES;
// 	FShaderResourceParameter	GFSDK_HAIR_RESOURCE_TANGENTS;
// 	FShaderResourceParameter	GFSDK_HAIR_RESOURCE_NORMALS;
// 	FShaderResourceParameter	GFSDK_HAIR_RESOURCE_MASTER_POSITIONS;
// 	FShaderResourceParameter	GFSDK_HAIR_RESOURCE_MASTER_PREV_POSITIONS;
// };


