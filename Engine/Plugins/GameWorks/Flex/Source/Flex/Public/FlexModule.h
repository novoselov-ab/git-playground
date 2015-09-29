#pragma once

#include "ModuleInterface.h"

class FFlexModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;

	virtual void ShutdownModule() override;

private:

	void * CudaRtHandle = nullptr;
	void * FLEXCoreHandle = nullptr;
	void * FLEXExtHandle = nullptr;

};

