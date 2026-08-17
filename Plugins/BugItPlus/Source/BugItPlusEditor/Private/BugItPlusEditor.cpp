#include "BugItPlusEditor.h"
#include "BugItPlus.h"
#include "Editor.h"
#include "FileHelpers.h"
#include "Engine/World.h"
#include "LevelEditorViewport.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

#define LOCTEXT_NAMESPACE "FBugItPlusEditorModule"

void FBugItPlusEditorModule::StartupModule()
{
	FBugItPlusModule::EditorCaptureDelegate.BindStatic(&FBugItPlusEditorModule::HandleEditorCapture);
	FBugItPlusModule::EditorJumpDelegate.BindStatic(&FBugItPlusEditorModule::HandleEditorJump);
	FBugItPlusModule::EditorNotifyDelegate.BindStatic(&FBugItPlusEditorModule::HandleEditorNotify);
}

void FBugItPlusEditorModule::ShutdownModule()
{
	FBugItPlusModule::EditorCaptureDelegate.Unbind();
	FBugItPlusModule::EditorJumpDelegate.Unbind();
	FBugItPlusModule::EditorNotifyDelegate.Unbind();
}

bool FBugItPlusEditorModule::HandleEditorCapture(FString& OutMapPackageName, FTransform& OutTransform)
{
	if (!GCurrentLevelEditingViewportClient || !GEditor)
	{
		return false;
	}

	UWorld* EditorWorld = GEditor->GetEditorWorldContext().World();
	if (!EditorWorld)
	{
		return false;
	}

	OutMapPackageName = EditorWorld->GetOutermost()->GetName();
	OutTransform = FTransform(GCurrentLevelEditingViewportClient->GetViewRotation(), GCurrentLevelEditingViewportClient->GetViewLocation());

	return true;
}

bool FBugItPlusEditorModule::HandleEditorJump(const FString& MapPackageName, const FTransform& Transform)
{
	if (!GEditor)
	{
		return false;
	}


	const UWorld* EditorWorld = GEditor->GetEditorWorldContext().World();
	const FString CurrentMapPackageName = EditorWorld ? EditorWorld->GetOutermost()->GetName() : FString();

	if (CurrentMapPackageName != MapPackageName)
	{
		if (!FEditorFileUtils::LoadMap(MapPackageName, /*LoadAsTemplate=*/false, /*bShowProgress=*/true))
		{
			UE_LOG(LogTemp, Warning, TEXT("BugItGoPlus: failed to load map '%s'"), *MapPackageName);
			return false;
		}
	}

	const FVector Location = Transform.GetLocation();
	FRotator Rotation = Transform.Rotator();
	Rotation.Roll = 0.0f;

	for (FLevelEditorViewportClient* ViewportClient : GEditor->GetLevelViewportClients())
	{
		ViewportClient->SetViewLocation(Location);
		ViewportClient->SetViewRotation(Rotation);
	}

	GEditor->RedrawLevelEditingViewports();

	return true;
}

void FBugItPlusEditorModule::HandleEditorNotify(const FString& Message)
{
	FNotificationInfo Info(FText::FromString(Message));
	Info.ExpireDuration = 5.0f;
	FSlateNotificationManager::Get().AddNotification(Info);
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FBugItPlusEditorModule, BugItPlusEditor)