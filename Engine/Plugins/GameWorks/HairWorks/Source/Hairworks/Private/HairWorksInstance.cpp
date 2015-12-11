#include "HairworksPCH.h"

FHairWorksInstance::FHairWorksInstance():
	Hair(nullptr),
	HairMaterial(nullptr)
{
	HairMaterial = NewObject<UHairWorksMaterial>(GetTransientPackage(), TEXT("HairMaterial"), RF_Public);
}
