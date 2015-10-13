#include "FlexPCH.h"

IMPLEMENT_MODULE(FFlexModule, Flex);
DEFINE_LOG_CATEGORY(LogFlex);


void FFlexModule::StartupModule()
{
	FString DllPath = FPaths::EngineDir() / TEXT("Plugins/GameWorks/Flex/Libraries/Win64/");

	auto loadDll = [](FString rootPath, FString dll) -> void * {
		FPlatformProcess::PushDllDirectory(*rootPath);
		auto dllHandle = FPlatformProcess::GetDllHandle(*(rootPath + dll));
		FPlatformProcess::PopDllDirectory(*rootPath);

		return dllHandle;
	};

#if PLATFORM_64BITS
	DllPath = DllPath / TEXT("Win64/");

	#if UE_BUILD_DEBUG && !defined(NDEBUG)	// Use !defined(NDEBUG) to check to see if we actually are linking with Debug third party libraries (bDebugBuildsActuallyUseDebugCRT)
		CudaRtHandle = loadDll(DllPath, "cudart64_70.dll");
		FLEXCoreHandle = loadDll(DllPath, "flexDebug_x64.dll");
		FLEXExtHandle = loadDll(DllPath, "flexExtDebug_x64.dll");
	#elif (UE_BUILD_SHIPPING || UE_BUILD_TEST) && SHIPPING_BUILDS_ACTUALLY_USE_SHIPPING_PHYSX_LIBRARIES
		CudaRtHandle = loadDll(DllPath, "cudart64_70.dll");
		FLEXCoreHandle = loadDll(DllPath, "flexRelease_x64.dll");
		FLEXExtHandle = loadDll(DllPath, "flexExtRelease_x64.dll");
	#else
		CudaRtHandle = loadDll(DllPath, "cudart64_70.dll");
		FLEXCoreHandle = loadDll(DllPath, "flexRelease_x64.dll");
		FLEXExtHandle = loadDll(DllPath, "flexExtRelease_x64.dll");
	#endif

#else
	DllPath = DllPath / TEXT("Win32/");

	#if UE_BUILD_DEBUG && !defined(NDEBUG)	// Use !defined(NDEBUG) to check to see if we actually are linking with Debug third party libraries (bDebugBuildsActuallyUseDebugCRT)
		CudaRtHandle = loadDll(DllPath, "cudart32_70.dll");
		FLEXCoreHandle = loadDll(DllPath, "flexDebug_x86.dll");
		FLEXExtHandle = loadDll(DllPath, "flexExtDebug_x86.dll");
	#elif (UE_BUILD_SHIPPING || UE_BUILD_TEST) && SHIPPING_BUILDS_ACTUALLY_USE_SHIPPING_PHYSX_LIBRARIES
		CudaRtHandle = loadDll(DllPath, "cudart32_70.dll");
		FLEXCoreHandle = loadDll(DllPath, "flexRelease_x86.dll");
		FLEXExtHandle = loadDll(DllPath, "flexExtRelease_x86.dll");
	#else
		CudaRtHandle = loadDll(DllPath, "cudart32_70.dll");
		FLEXCoreHandle = loadDll(DllPath, "flexRelease_x86.dll");
		FLEXExtHandle = loadDll(DllPath, "flexExtRelease_x86.dll");
	#endif

#endif

	check(CudaRtHandle);
	check(FLEXCoreHandle);
	check(FLEXExtHandle);
}

void FFlexModule::ShutdownModule()
{
	if (CudaRtHandle != nullptr)
	{
		FPlatformProcess::FreeDllHandle(CudaRtHandle);
		CudaRtHandle = nullptr;
	}

	if (FLEXCoreHandle != nullptr)
	{
		FPlatformProcess::FreeDllHandle(FLEXCoreHandle);
		FLEXCoreHandle = nullptr;
	}

	if (FLEXExtHandle != nullptr)
	{
		FPlatformProcess::FreeDllHandle(FLEXExtHandle);
		FLEXExtHandle = nullptr;
	}

}
