// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "MovieSceneCaptureSettings.h"
#include "IMovieSceneCapture.h"
#include "MovieSceneCaptureHandle.h"
#include "MovieScene.h"
#include "AVIWriter.h"
#include "MovieSceneCapture.generated.h"

/** Interface that defines when to capture or drop frames */
struct ICaptureStrategy
{
	virtual ~ICaptureStrategy(){}
	
	virtual void OnStart() = 0;
	virtual void OnStop() = 0;
	virtual void OnPresent(double CurrentTimeSeconds, uint32 FrameIndex) = 0;
	virtual bool ShouldSynchronizeFrames() const { return true; }

	virtual bool ShouldPresent(double CurrentTimeSeconds, uint32 FrameIndex) const = 0;
	virtual int32 GetDroppedFrames(double CurrentTimeSeconds, uint32 FrameIndex) const = 0;
};

/** Structure used to cache various metrics for our capture */
struct FCachedMetrics
{
	FCachedMetrics() : Width(0), Height(0), Frame(0), ElapsedSeconds(0.f) {}

	/** The width/Height of the frame */
	int32 Width, Height;
	/** The current frame number */
	int32 Frame;
	/** The number of seconds that have elapsed */
	float ElapsedSeconds;
};

/** Class responsible for capturing scene data */
UCLASS(config=EditorSettings)
class MOVIESCENECAPTURE_API UMovieSceneCapture : public UObject, public IMovieSceneCaptureInterface
{
public:

	UMovieSceneCapture(const FObjectInitializer& Initializer);

	GENERATED_BODY()

public:

	// Begin IMovieSceneCaptureInterface
	virtual void Initialize(FViewport* InViewport) override;
	virtual void Close() override;
	virtual FMovieSceneCaptureHandle GetHandle() const override { return Handle; }
	const FMovieSceneCaptureSettings& GetSettings() const override { return Settings; }
	// End IMovieSceneCaptureInterface

public:

	/** Settings that define how to capture */
	UPROPERTY(EditAnywhere, config, Category=CaptureSettings, meta=(ShowOnlyInnerProperties))
	FMovieSceneCaptureSettings Settings;

	/** Additional command line arguments to pass to the external process when capturing */
	UPROPERTY(EditAnywhere, transient, Category=General, AdvancedDisplay)
	FString AdditionalCommandLineArguments;

	/** Value used to control the BufferVisualizationDumpFrames cvar in the child process */
	UPROPERTY()
	bool bBufferVisualizationDumpFrames;

public:

	/** Access this object's cached metrics */
	const FCachedMetrics& GetMetrics() const { return CachedMetrics; }

	/** Initialize the capture */
	void StartCapture();

	/** Finalize the capture, waiting for any outstanding processing */
	void StopCapture();

	/** Capture a frame from our bound viewport */
	virtual void CaptureFrame(float DeltaSeconds);

protected:

#if WITH_EDITOR
	/** Implementation function that saves out a snapshot file from the specified color data */
	void CaptureSnapshot(const TArray<FColor>& Colors);
#endif

protected:

	/** Resolve the specified format using the user supplied formatting rules. */
	FString ResolveFileFormat(const FString& Folder, const FString& Format) const;

	/** Get the default extension required for this capture (avi, jpg etc) */
	const TCHAR* GetDefaultFileExtension() const;

	/** Calculate a unique index for the {unique} formatting rule */
	FString ResolveUniqueFilename();

protected:
	
	/** The viewport we are bound to */
	FViewport* Viewport;
	/** Our unique handle, used for external representation without having to link to the MovieSceneCapture module */
	FMovieSceneCaptureHandle Handle;
	/** Cached metrics for this capture operation */
	FCachedMetrics CachedMetrics;
	/** Optional AVI file writer used when capturing to video */
	TUniquePtr<FAVIWriter> AVIWriter;
	/** Strategy used for capture (real-time/fixed-time-step) */
	TSharedPtr<ICaptureStrategy> CaptureStrategy;
};

/** A strategy that employs a fixed frame time-step, and as such never drops a frame. Potentially accelerated.  */
struct MOVIESCENECAPTURE_API FFixedTimeStepCaptureStrategy : ICaptureStrategy
{
	FFixedTimeStepCaptureStrategy(uint32 InTargetFPS);

	virtual void OnStart() override;
	virtual void OnStop() override;
	virtual void OnPresent(double CurrentTimeSeconds, uint32 FrameIndex) override;	
	virtual bool ShouldPresent(double CurrentTimeSeconds, uint32 FrameIndex) const override;
	virtual int32 GetDroppedFrames(double CurrentTimeSeconds, uint32 FrameIndex) const override;

private:
	uint32 TargetFPS;
};

/** A capture strategy that captures in real-time, potentially dropping frames to maintain a stable constant framerate video. */
struct MOVIESCENECAPTURE_API FRealTimeCaptureStrategy : ICaptureStrategy
{
	FRealTimeCaptureStrategy(uint32 InTargetFPS);

	virtual void OnStart() override;
	virtual void OnStop() override;
	virtual void OnPresent(double CurrentTimeSeconds, uint32 FrameIndex) override;
	virtual bool ShouldSynchronizeFrames() const override { return false; }
	virtual bool ShouldPresent(double CurrentTimeSeconds, uint32 FrameIndex) const override;
	virtual int32 GetDroppedFrames(double CurrentTimeSeconds, uint32 FrameIndex) const override;

private:
	double NextPresentTimeS, FrameLength;
};