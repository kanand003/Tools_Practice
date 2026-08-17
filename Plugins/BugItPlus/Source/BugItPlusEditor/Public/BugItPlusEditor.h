#pragma once

#include "CoreMinimal.h"

class FBugItPlusEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
private:
    static bool HandleEditorCapture(FString& OutMapPackageName, FTransform& OutTransform);
    static bool HandleEditorJump(const FString& MapPackageName, const FTransform& Transform);
    static void HandleEditorNotify(const FString& Message);
};
