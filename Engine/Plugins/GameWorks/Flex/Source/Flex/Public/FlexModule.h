#pragma once

#include "ModuleInterface.h"

class FFlexModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;

	virtual void ShutdownModule() override;

private:
};

