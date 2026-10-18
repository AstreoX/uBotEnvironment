#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

UBOTENVIRONMENT_API DECLARE_LOG_CATEGORY_EXTERN(LogUBotEnvironment, Log, All);

class FUBotEnvironmentModule final : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};
