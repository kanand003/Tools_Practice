#include "BugItPlus.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBugItPlusEditorCaptureDelegateTest, "BugItPlus.Delegates.EditorCapture",
                                 EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FBugItPlusEditorCaptureDelegateTest::RunTest(const FString& Parameters)
{
	const FBugItPlusEditorCaptureDelegate SavedDelegate = FBugItPlusModule::EditorCaptureDelegate;

	FBugItPlusModule::EditorCaptureDelegate.Unbind();
	TestFalse(TEXT("Unbound after explicit Unbind"), FBugItPlusModule::EditorCaptureDelegate.IsBound());

	FBugItPlusModule::EditorCaptureDelegate.BindLambda([](FString& OutMapPackageName, FTransform& OutTransform)
	{
		OutMapPackageName = TEXT("/Game/Maps/TestMap");
		OutTransform = FTransform::Identity;
		return true;
	});

	TestTrue(TEXT("Bound after BindLambda"), FBugItPlusModule::EditorCaptureDelegate.IsBound());

	FString MapPackageName;
	FTransform Transform;
	const bool bResult = FBugItPlusModule::EditorCaptureDelegate.Execute(MapPackageName, Transform);
	TestTrue(TEXT("Execute returns true"), bResult);

	TestEqual(TEXT("Execute fills map package name"), MapPackageName, FString(TEXT("/Game/Maps/TestMap")));

	FBugItPlusModule::EditorCaptureDelegate = SavedDelegate;

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBugItPlusEditorJumpDelegateTest, "BugItPlus.Delegates.EditorJump",
                                 EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FBugItPlusEditorJumpDelegateTest::RunTest(const FString& Parameters)
{
	const FBugItPlusEditorJumpDelegate SavedDelegate = FBugItPlusModule::EditorJumpDelegate;

	FBugItPlusModule::EditorJumpDelegate.Unbind();
	TestFalse(TEXT("Unbound after explicit Unbind"), FBugItPlusModule::EditorJumpDelegate.IsBound());

	bool bWasCalled = false;
	FBugItPlusModule::EditorJumpDelegate.BindLambda(
		[&bWasCalled](const FString& MapPackageName, const FTransform& Transform)
		{
			bWasCalled = true;
			return true;
		});

	TestTrue(TEXT("Bound after BindLambda"), FBugItPlusModule::EditorJumpDelegate.IsBound());

	const bool bResult = FBugItPlusModule::EditorJumpDelegate.Execute(TEXT("/Game/Maps/TestMap"), FTransform::Identity);

	TestTrue(TEXT("Execute returns true"), bResult);
	TestTrue(TEXT("Bound lambda was invoked"), bWasCalled);

	FBugItPlusModule::EditorJumpDelegate = SavedDelegate;

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
