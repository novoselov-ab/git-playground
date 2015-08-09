#include "GWGluePCH.h"
#include "ModuleInterface.h"

class FGWGlueModule : public FDefaultModuleImpl
{
public:
	virtual void StartupModule() override
	{
	}


	virtual void ShutdownModule() override
	{
	}
};

IMPLEMENT_MODULE(FGWGlueModule, GWGlue);
