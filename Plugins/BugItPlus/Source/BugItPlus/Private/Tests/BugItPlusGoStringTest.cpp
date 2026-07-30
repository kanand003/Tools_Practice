// Fill out your copyright notice in the Description page of Project Settings.


#include "BugItPlusGoString.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBugItPlusGoStringBuildTest, "BugItPlus.GoString.Build", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FBugItPlusGoStringBuildTest::RunTest(const FString& Parameters)
{
      const FString Result = FBugItPlusGoString::Build(TEXT("/Game/Maps/ThirdPersonMap"), FVector(100.0f, 200.0f, 300.0f), FRotator(10.0f, 20.0f, 30.0f));

      TestEqual(TEXT("Go-string matches expected format"), Result, FString(TEXT("/Game/Maps/ThirdPersonMap 100.000000 200.000000 300.000000 10.000000 20.000000 30.000000")));

      return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBugItPlusGoStringParseRoundTripTest, "BugItPlus.GoString.ParseRoundTrip", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FBugItPlusGoStringParseRoundTripTest::RunTest(const FString& Parameters)
{
      const FString MapPackageName = TEXT("/Game/Maps/ThirdPersonMap");
      const FVector Location(100.0f, 200.0f, 300.0f);
      const FRotator Rotation(10.0f, 20.0f, 30.0f);

      const FString GoString = FBugItPlusGoString::Build(MapPackageName, Location, Rotation);

      FString ParsedMapPackageName;
      FVector ParsedLocation;
      FRotator ParsedRotation;
      const bool bParsed = FBugItPlusGoString::Parse(GoString, ParsedMapPackageName, ParsedLocation, ParsedRotation);

      TestTrue(TEXT("Parse succeeds"), bParsed);
      TestEqual(TEXT("Map package name round-trips"), ParsedMapPackageName, MapPackageName);
      TestEqual(TEXT("Location round-trips"), ParsedLocation, Location);
      TestEqual(TEXT("Rotation round-trips"), ParsedRotation, Rotation);

      return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBugItPlusGoStringParseRejectsMalformedTest, "BugItPlus.GoString.ParseRejectsMalformed", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FBugItPlusGoStringParseRejectsMalformedTest::RunTest(const FString& Parameters)
{
      FString ParsedMapPackageName;
      FVector ParsedLocation;
      FRotator ParsedRotation;

      const bool bParsed = FBugItPlusGoString::Parse(TEXT("not enough tokens"), ParsedMapPackageName, ParsedLocation, ParsedRotation);

      TestFalse(TEXT("Parse rejects a malformed go-string"), bParsed);

      return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS