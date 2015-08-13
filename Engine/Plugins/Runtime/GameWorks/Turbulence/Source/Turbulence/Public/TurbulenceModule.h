#pragma once
#include "ModuleInterface.h"

class FTurbulenceModule : IModuleInterface
{
public:

	virtual void StartupModule() override;

	virtual void ShutdownModule() override;
};