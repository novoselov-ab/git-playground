#include "HairworksPCH.h"

FHairProperties::FHairProperties() :
SplineMultiplier(0),
bSimulate(true),
MassScale(10.0f),
Damping(0),
InertiaScale(1.0f),
InertiaLimit(100.0f),
WindDirection(EForceInit::ForceInitToZero),
Wind(0),
WindNoise(0),
StiffnessGlobal(0.5f),
StiffnessGlobalMap(nullptr),
StiffnessGlobalCurve(1.0f, 1.0f, 1.0f, 1.0f),
StiffnessStrength(1.0f),
StiffnessStrengthCurve(1.0f, 1.0f, 1.0f, 1.0f),
StiffnessDamping(0),
StiffnessDampingCurve(1.0f, 1.0f, 1.0f, 1.0f),
StiffnessRoot(0),
StiffnessRootMap(nullptr),
StiffnessTip(0),
StiffnessBend(0),
StiffnessBendCurve(1.0f, 1.0f, 1.0f, 1.0f),
Backstop(0),
Friction(0),
bCapsuleCollision(false),
StiffnessInteraction(0),
StiffnessInteractionCurve(1.0f, 1.0f, 1.0f, 1.0f),
PinStiffness(1.0f),
Density(1.0f),
bUsePixelDensity(false),
LengthScale(1.0f),
LengthScaleMap(nullptr),
LengthNoise(0),
WidthScale(1.0f),
WidthRootScale(1.0f),
WidthTipScale(0.1f),
WidthNoise(0),
ClumpingScale(0),
ClumpingScaleMap(nullptr),
ClumpingRoundness(1.0f),
ClumpingRoundnessMap(nullptr),
ClumpingNoise(0),
WavinessScale(0),
WavinessScaleMap(nullptr),
WavinessScaleNoise(0.5f),
WavinessScaleClump(0),
WavinessFreq(3.0f),
WavinessRootStraigthen(0),
RootColor(1.0f, 1.0f, 1.0f, 1.0f),
RootColorMap(nullptr),
TipColor(1.0f, 1.0f, 1.0f, 1.0f),
TipColorMap(nullptr),
RootTipColorWeight(0.5f),
RootTipColorFalloff(0),
RootAlphaFalloff(0),
DiffuseBlend(0.5f),
HairNormalWeight(0),
HairNormalCenter(NAME_None),
SpecularColor(1.0f, 1.0f, 1.0f, 1.0f),
SpecularColorMap(nullptr),
PrimaryScale(0.1f),
PrimaryShininess(100.0f),
PrimaryBreakup(0),
SecondaryScale(0.05f),
SecondaryShininess(20.0f),
SecondaryOffset(0.1f),
GlintStrength(0),
GlintSize(256.0f),
GlintPowerExponent(2.0f),
ShadowDensityScale(0.5f),
bBackfaceCulling(false),
BackfaceCullingThreshold(-0.2f),
bDistanceLodEnable(false),
DistanceLodStart(5.0f),
DistanceLodEnd(10.0f),
FadeStartDistance(1000.0f),
DistanceLodBaseWidthScale(1.0f),
DistanceLodBaseDensityScale(0),
bDetailLodEnable(false),
DetailLodStart(2.0f),
DetailLodEnd(1.0f),
DetailLodBaseWidthScale(10.0f),
DetailLodBaseDensityScale(1.0f)
{

}

void FHairProperties::ImportPropertiesFrom(const GFSDK_HairInstanceDescriptor& Descriptor)
{

}

void FHairProperties::ExportPropertiesTo(GFSDK_HairInstanceDescriptor& Descriptor)
{
	GetTextures(); // Make sure this is up to date.



}

const TArray<FTexture2DRHIRef>& FHairProperties::GetTextures()
{
	if (CachedTextures.Num() == 0)
	{
		CachedTextures.SetNumZeroed(GFSDK_HAIR_NUM_TEXTURES);
	}

	auto CastTexture = [](UTexture2D* texture) -> FTexture2DRHIRef 
	{ 
		return texture != nullptr ? static_cast<FTexture2DResource *>(texture->Resource)->GetTexture2DRHI() : nullptr;
	};

	CachedTextures[GFSDK_HAIR_TEXTURE_DENSITY] = CastTexture(DensityMap);
	CachedTextures[GFSDK_HAIR_TEXTURE_ROOT_COLOR] = CastTexture(RootColorMap);
	CachedTextures[GFSDK_HAIR_TEXTURE_TIP_COLOR] = CastTexture(TipColorMap);
	CachedTextures[GFSDK_HAIR_TEXTURE_WIDTH] = CastTexture(WidthScaleMap);
	CachedTextures[GFSDK_HAIR_TEXTURE_STIFFNESS] = CastTexture(StiffnessGlobalMap);
	CachedTextures[GFSDK_HAIR_TEXTURE_ROOT_STIFFNESS] = CastTexture(StiffnessRootMap);
	CachedTextures[GFSDK_HAIR_TEXTURE_CLUMP_SCALE] = CastTexture(ClumpingScaleMap);
	CachedTextures[GFSDK_HAIR_TEXTURE_CLUMP_ROUNDNESS] = CastTexture(ClumpingRoundnessMap);
	CachedTextures[GFSDK_HAIR_TEXTURE_WAVE_SCALE] = CastTexture(WavinessScaleMap);
	CachedTextures[GFSDK_HAIR_TEXTURE_WAVE_FREQ] = CastTexture(WavinessFreqMap);
	CachedTextures[GFSDK_HAIR_TEXTURE_STRAND] = nullptr;
	CachedTextures[GFSDK_HAIR_TEXTURE_LENGTH] = CastTexture(LengthScaleMap);
	CachedTextures[GFSDK_HAIR_TEXTURE_SPECULAR] = CastTexture(SpecularColorMap);
	CachedTextures[GFSDK_HAIR_TEXTURE_WEIGHTS] = nullptr;

	return CachedTextures;
}
