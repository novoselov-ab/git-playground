#include "HairworksPCH.h"

#include "GlobalShader.h"
#include "ShaderParameters.h"
#include "ShaderParameterUtils.h"
#include "RHIStaticStates.h"

// Pixel shaders
IMPLEMENT_SHADER_TYPE(, FHairWorksPs, TEXT("HairWorks"), TEXT("Main"), SF_Pixel);
// IMPLEMENT_SHADER_TYPE(, FHairWorksVelocityPs, TEXT("HairWorks"), TEXT("VelocityMain"), SF_Pixel);
// IMPLEMENT_SHADER_TYPE(, FHairWorksSimplePs, TEXT("HairWorks"), TEXT("SimpleMain"), SF_Pixel);
IMPLEMENT_SHADER_TYPE(, FHairWorksShadowDepthPs, TEXT("HairWorks"), TEXT("ShadowDepthMain"), SF_Pixel);

FHairWorksPs::FHairWorksPs(const ShaderMetaType::CompiledShaderInitializerType& Initializer) : FGlobalShader(Initializer)
{
	HairConstantBuffer.Bind(Initializer.ParameterMap, TEXT("HairConstantBuffer"));

	TextureSampler.Bind(Initializer.ParameterMap, TEXT("TextureSampler"));

	RootColorTexture.Bind(Initializer.ParameterMap, TEXT("RootColorTexture"));
	TipColorTexture.Bind(Initializer.ParameterMap, TEXT("TipColorTexture"));
	SpecularColorTexture.Bind(Initializer.ParameterMap, TEXT("SpecularColorTexture"));

	GFSDK_HAIR_RESOURCE_FACE_HAIR_INDICES.Bind(Initializer.ParameterMap, TEXT("GFSDK_HAIR_RESOURCE_FACE_HAIR_INDICES"));
	GFSDK_HAIR_RESOURCE_TANGENTS.Bind(Initializer.ParameterMap, TEXT("GFSDK_HAIR_RESOURCE_TANGENTS"));
	GFSDK_HAIR_RESOURCE_NORMALS.Bind(Initializer.ParameterMap, TEXT("GFSDK_HAIR_RESOURCE_NORMALS"));
}

FHairWorksPs::FHairWorksPs()
{

}

bool FHairWorksPs::Serialize(FArchive& Ar)
{
	bool bShaderHasOutdatedParameters = FGlobalShader::Serialize(Ar);

	Ar << HairConstantBuffer << TextureSampler << RootColorTexture << TipColorTexture << SpecularColorTexture << GFSDK_HAIR_RESOURCE_FACE_HAIR_INDICES << GFSDK_HAIR_RESOURCE_TANGENTS << GFSDK_HAIR_RESOURCE_NORMALS;

	return bShaderHasOutdatedParameters;
}

void FHairWorksPs::SetParameters(FRHICommandListImmediate& RHICmdList, const FSceneView& View, const GFSDK_Hair_ConstantBuffer& HairConstBuffer, const TArray<FTexture2DRHIRef>& HairTextures )
{
	FGlobalShader::SetParameters(RHICmdList, GetPixelShader(), View);

	SetShaderValue(RHICmdList, GetPixelShader(), this->HairConstantBuffer, HairConstBuffer);

	SetSamplerParameter(RHICmdList, GetPixelShader(), TextureSampler, TStaticSamplerState<>::GetRHI());

	SetTextureParameter(RHICmdList, GetPixelShader(), RootColorTexture, HairTextures[GFSDK_HAIR_TEXTURE_ROOT_COLOR]);
	SetTextureParameter(RHICmdList, GetPixelShader(), TipColorTexture, HairTextures[GFSDK_HAIR_TEXTURE_TIP_COLOR]);
	SetTextureParameter(RHICmdList, GetPixelShader(), SpecularColorTexture, HairTextures[GFSDK_HAIR_TEXTURE_SPECULAR]);
}

bool FHairWorksPs::ShouldCache(EShaderPlatform Platform)
{
	return IsFeatureLevelSupported(Platform, ERHIFeatureLevel::SM5);
}

void FHairWorksPs::ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment)
{
	OutEnvironment.SetDefine(TEXT("MATERIALBLENDING_SOLID"), TEXT("1"));

	FGlobalShader::ModifyCompilationEnvironment(Platform, OutEnvironment);
}







// 
// FHairWorksSimplePs::FHairWorksSimplePs(const ShaderMetaType::CompiledShaderInitializerType& Initializer) : FGlobalShader(Initializer)
// {
// 
// }
// 
// FHairWorksSimplePs::FHairWorksSimplePs()
// {
// 
// }
// 
// bool FHairWorksSimplePs::ShouldCache(EShaderPlatform Platform)
// {
// 	return IsFeatureLevelSupported(Platform, ERHIFeatureLevel::SM5);
// }
// 
// void FHairWorksSimplePs::ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment)
// {
// 	FGlobalShader::ModifyCompilationEnvironment(Platform, OutEnvironment);
// }
// 
FHairWorksShadowDepthPs::FHairWorksShadowDepthPs(const ShaderMetaType::CompiledShaderInitializerType& Initializer) : FGlobalShader(Initializer)
{
	ShadowParams.Bind(Initializer.ParameterMap, TEXT("ShadowParams"));
}

FHairWorksShadowDepthPs::FHairWorksShadowDepthPs()
{

}

bool FHairWorksShadowDepthPs::Serialize(FArchive& Ar)
{
	bool bSerialized = FGlobalShader::Serialize(Ar);
	Ar << ShadowParams;
	return bSerialized;
}

bool FHairWorksShadowDepthPs::ShouldCache(EShaderPlatform Platform)
{
	return IsFeatureLevelSupported(Platform, ERHIFeatureLevel::SM5);
}

void FHairWorksShadowDepthPs::ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment)
{
	FGlobalShader::ModifyCompilationEnvironment(Platform, OutEnvironment);
}
// 
// FHairWorksVelocityPs::FHairWorksVelocityPs(const ShaderMetaType::CompiledShaderInitializerType& Initializer) : FGlobalShader(Initializer)
// {
// 	HairConstantBuffer.Bind(Initializer.ParameterMap, TEXT("HairConstantBuffer"));
// 
// 	GFSDK_HAIR_RESOURCE_FACE_HAIR_INDICES.Bind(Initializer.ParameterMap, TEXT("GFSDK_HAIR_RESOURCE_FACE_HAIR_INDICES"));
// 	GFSDK_HAIR_RESOURCE_TANGENTS.Bind(Initializer.ParameterMap, TEXT("GFSDK_HAIR_RESOURCE_TANGENTS"));
// 	GFSDK_HAIR_RESOURCE_NORMALS.Bind(Initializer.ParameterMap, TEXT("GFSDK_HAIR_RESOURCE_NORMALS"));
// 	GFSDK_HAIR_RESOURCE_MASTER_POSITIONS.Bind(Initializer.ParameterMap, TEXT("GFSDK_HAIR_RESOURCE_MASTER_POSITIONS"));
// 	GFSDK_HAIR_RESOURCE_MASTER_PREV_POSITIONS.Bind(Initializer.ParameterMap, TEXT("GFSDK_HAIR_RESOURCE_MASTER_PREV_POSITIONS"));
// }
// 
// FHairWorksVelocityPs::FHairWorksVelocityPs()
// {
// 
// }
// 
// bool FHairWorksVelocityPs::Serialize(FArchive& Ar)
// {
// 	bool bShaderHasOutdatedParameters = FGlobalShader::Serialize(Ar);
// 
// 	Ar << HairConstantBuffer << GFSDK_HAIR_RESOURCE_FACE_HAIR_INDICES << GFSDK_HAIR_RESOURCE_TANGENTS << GFSDK_HAIR_RESOURCE_NORMALS << GFSDK_HAIR_RESOURCE_MASTER_POSITIONS << GFSDK_HAIR_RESOURCE_MASTER_PREV_POSITIONS;
// 
// 	return bShaderHasOutdatedParameters;
// }
// 
// bool FHairWorksVelocityPs::ShouldCache(EShaderPlatform Platform)
// {
// 	return IsFeatureLevelSupported(Platform, ERHIFeatureLevel::SM5);
// }
// 
// void FHairWorksVelocityPs::ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment)
// {
// 	FGlobalShader::ModifyCompilationEnvironment(Platform, OutEnvironment);
// }
