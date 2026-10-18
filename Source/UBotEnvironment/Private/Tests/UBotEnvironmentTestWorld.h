#pragma once

#if WITH_DEV_AUTOMATION_TESTS

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"

namespace UBotEnvironmentTests
{
    // A transient game world with actors initialized for play; no dependency on a consumer plugin.
    class FScopedEnvironmentTestWorld
    {
    public:
        UE_NONCOPYABLE(FScopedEnvironmentTestWorld);

        FScopedEnvironmentTestWorld()
            : World(UWorld::CreateWorld(EWorldType::Game, false))
        {
            if (World && GEngine)
            {
                World->AddToRoot();
                GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
                World->InitializeActorsForPlay(FURL());
            }
        }

        ~FScopedEnvironmentTestWorld()
        {
            if (!World)
            {
                return;
            }
            if (World->AreActorsInitialized())
            {
                for (TActorIterator<AActor> It(World); It; ++It)
                {
                    It->RouteEndPlay(EEndPlayReason::LevelTransition);
                }
            }
            if (GEngine)
            {
                GEngine->DestroyWorldContext(World);
            }
            World->DestroyWorld(false);
            World->RemoveFromRoot();
        }

        UWorld* Get() const { return World; }

        template <typename T>
        T* Spawn() const
        {
            FActorSpawnParameters Parameters;
            Parameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
            return World->SpawnActor<T>(FVector::ZeroVector, FRotator::ZeroRotator, Parameters);
        }

    private:
        UWorld* World = nullptr;
    };
}

#endif
