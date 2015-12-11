#include "HairworksPCH.h"

//JDM: Convert to singleton, UE4 style.
FHairWorksManager* GHairManager = nullptr;

IMPLEMENT_MODULE( FHairworksModule, HairWorks );
DEFINE_LOG_CATEGORY(LogHairWorks);

void FHairworksModule::StartupModule()
{
	// Get it, which causes it to init.
	HairManager = MakeUnique<FHairWorksManager>();
	
	// Not happy about exposing this raw pointer
	GHairManager = HairManager.Get();
}

void FHairworksModule::ShutdownModule()
{
}
