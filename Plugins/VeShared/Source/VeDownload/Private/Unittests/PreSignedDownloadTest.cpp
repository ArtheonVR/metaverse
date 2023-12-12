﻿// Author: Egor A. Pristavka. 
// Copyright (c) 2022-2023 VeVerse AS.
// Copyright (c) 2023 LE7EL AS. All rights reserved.

#include "Download1.h"
#include "IVeBaseLatentCommand.h"
#include "VeDownload.h"
#include "Misc/AutomationTest.h"
#include "Download1Subsystem.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPreSignedDownloadTest, "Game.VeDownload.PreSignedDownloadTest", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

class FTestPreSignedDownloadLatentCommand final : public IVeBaseLatentCommand {
	TArray<FString> TestUrls = {
		""
	};

	TArray<TSharedPtr<FVeDownload1>> Downloads;
	int32 ProcessedUrls = 0;
	int32 LastReported = -1;

public:
	explicit FTestPreSignedDownloadLatentCommand(FAutomationTestBase* InTest, IVeBaseLatentCommand* InParentCommand = nullptr, const double InTimeout = 30.0)
		: IVeBaseLatentCommand(InTest, InParentCommand, InTimeout) { }

	virtual ~FTestPreSignedDownloadLatentCommand() override { }

	virtual bool Update() override {
		if (const int32 Current = TestUrls.Num() - ProcessedUrls; LastReported != Current) {
			Test->AddInfo(FString::Printf(TEXT("processed %d of %d, remaining %d"), ProcessedUrls, TestUrls.Num(), Current));
			LastReported = Current;
		}
		return IVeBaseLatentCommand::Update();
	}

	virtual void Run() override {
		VeLogFunc("start");

		GET_MODULE_SUBSYSTEM(DownloadSubsystem, Download, Download1);
		Test->TestValid("download subsystem", DownloadSubsystem);
		if (!DownloadSubsystem) {
			VeLogErrorFunc("no download subsystem");
			SetComplete();
			return;
		}

		for (const auto& RequestUrl : TestUrls) {
			VeLogFunc("test download url: %s", *RequestUrl);
			FDownload1RequestMetadata RequestMetadata;
			FGuid::Parse("8DAC6786-7CCB-4DFB-9A14-7F4E857C3CDA", RequestMetadata.EntityId);
			FGuid::Parse("26B3455E-F04D-4D32-8BEE-ED426170102F", RequestMetadata.FileId);
			RequestMetadata.Mode = EDownload1Mode::Overwrite;
			RequestMetadata.bUseRange = false;
			RequestMetadata.bUsePreSignedUrl = true; // Should get the pre-signed URL automatically
			RequestMetadata.Priority = 0;

			const auto Download = DownloadSubsystem->Download(RequestMetadata);

			Download->GetOnDownloadProgress().AddLambda([=](const FDownload1ProgressMetadata& Metadata) {
				VeLogFunc("on progress: %s, %.3f", *RequestUrl, Metadata.Progress);
				Test->TestTrue(TEXT("Progress.TotalSize > 0"), Metadata.ContentLength > 0);
			});

			Download->GetOnDownloadComplete().AddLambda([=](const FDownload1ResultMetadata& Metadata) {
				VeLogFunc("on complete: %s", *RequestUrl);
				Test->TestTrue(TEXT("Result.Code == Completed"), Metadata.DownloadCode == EDownload1Code::Completed);

				IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
				const bool bFileExists = PlatformFile.FileExists(*Metadata.RequestMetadata.Path);
				Test->TestTrue(FString::Printf(TEXT("FileExists at %s"), *Metadata.RequestMetadata.Path), bFileExists);

				Test->TestTrue(FString::Printf(TEXT("FileSize is %lld"), Metadata.ContentLength), PlatformFile.FileSize(*Metadata.RequestMetadata.Path) == Metadata.ContentLength);
				Test->TestTrue(TEXT("Requested ContentLength matches actual ContentLength"), Metadata.RequestMetadata.ContentLength == Metadata.ContentLength);

				ProcessedUrls++;
				if (ProcessedUrls >= TestUrls.Num()) {
					SetComplete();
				}
			});
		}
	}
};

bool FPreSignedDownloadTest::RunTest(const FString& Parameters) {
	constexpr float LatentCommandTimeout = 300.0f;

	FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShared<FTestPreSignedDownloadLatentCommand>(this, nullptr, LatentCommandTimeout));

	// Make the test pass by returning true, or fail by returning false.
	return true;
}
