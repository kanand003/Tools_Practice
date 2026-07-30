#include "BugItPlusEditor.h"
#include "BugItPlus.h"
#include "Editor.h"
#include "LevelEditorViewport.h"
#include "Engine/World.h"

#define LOCTEXT_NAMESPACE "FBugItPlusEditorModule"

void FBugItPlusEditorModule::StartupModule()
{
	FBugItPlusModule::EditorCaptureDelegate.BindStatic(&FBugItPlusEditorModule::HandleEditorCapture);
	FBugItPlusModule::EditorJumpDelegate.BindStatic(&FBugItPlusEditorModule::HandleEditorJump);
}

void FBugItPlusEditorModule::ShutdownModule()
{
	FBugItPlusModule::EditorCaptureDelegate.Unbind();
	FBugItPlusModule::EditorJumpDelegate.Unbind();
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

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FBugItPlusEditorModule, BugItPlusEditor)