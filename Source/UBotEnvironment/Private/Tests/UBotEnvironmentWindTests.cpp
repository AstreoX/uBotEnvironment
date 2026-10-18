#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "UBotEnvironmentTestWorld.h"
#include "UBotWind.h"
#include "UBotWindQuerySubsystem.h"
#include "UBotWindSubsystem.h"

namespace UBotEnvironmentTests
{
    FUBotWindSettings MakeWind(const FVector& Constant, const FVector& Gust = FVector::ZeroVector, int32 Seed = 0)
    {
        FUBotWindSettings Settings;
        Settings.ConstantVelocityMetersPerSecond = Constant;
        Settings.GustStandardDeviationMetersPerSecond = Gust;
        Settings.Seed = Seed;
        return Settings;
    }

    AUBotWind* SpawnWind(const FScopedEnvironmentTestWorld& Scope, const FUBotWindSettings& Settings)
    {
        AUBotWind* Wind = Scope.Spawn<AUBotWind>();
        Wind->Settings = Settings;
        Wind->DispatchBeginPlay();
        return Wind;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUBotEnvironmentWindGustTest, "UBotEnvironment.Wind.GustDeterministic", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUBotEnvironmentWindGustTest::RunTest(const FString& Parameters)
{
    using namespace UBotEnvironmentTests;
    const FUBotWindSettings Settings = MakeWind(FVector::ZeroVector, FVector(1.0, 2.0, 0.5), 7);
    UBot::Wind::FGustModel A;
    UBot::Wind::FGustModel B;
    UBot::Wind::FGustModel Other;
    A.Build(Settings);
    B.Build(Settings);
    FUBotWindSettings OtherSettings = Settings;
    OtherSettings.Seed = 8;
    Other.Build(OtherSettings);

    bool bSame = true;
    bool bDiffers = false;
    FVector Sum = FVector::ZeroVector;
    FVector SumSquares = FVector::ZeroVector;
    constexpr int32 Samples = 60000;
    for (int32 Index = 0; Index < Samples; ++Index)
    {
        const double Time = Index * 0.01;
        const FVector Gust = A.Evaluate(Time);
        bSame &= Gust == B.Evaluate(Time);
        bDiffers |= !Gust.Equals(Other.Evaluate(Time), 1e-6);
        Sum += Gust;
        SumSquares += Gust * Gust;
    }
    TestTrue(TEXT("Same settings, same gusts"), bSame);
    TestTrue(TEXT("Another seed, other gusts"), bDiffers);
    for (int32 Axis = 0; Axis < 3; ++Axis)
    {
        const double Sigma = Settings.GustStandardDeviationMetersPerSecond[Axis];
        const double Mean = Sum[Axis] / Samples;
        const double Rms = FMath::Sqrt(SumSquares[Axis] / Samples);
        TestTrue(FString::Printf(TEXT("Axis %d: RMS %.3f within 15%% of %.3f"), Axis, Rms, Sigma), FMath::Abs(Rms - Sigma) <= 0.15 * Sigma);
        TestTrue(FString::Printf(TEXT("Axis %d: mean %.3f near zero"), Axis, Mean), FMath::Abs(Mean) <= 0.1 * Sigma);
    }

    UBot::Wind::FGustModel Calm;
    Calm.Build(MakeWind(FVector(3.0, 0.0, 0.0)));
    TestTrue(TEXT("No gusts without a deviation"), Calm.Evaluate(12.3).IsZero());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUBotEnvironmentWindSourcesTest, "UBotEnvironment.Wind.ActorAndScripted", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUBotEnvironmentWindSourcesTest::RunTest(const FString& Parameters)
{
    using namespace UBotEnvironmentTests;
    FScopedEnvironmentTestWorld Scope;
    UUBotWindSubsystem* Wind = Scope.Get()->GetSubsystem<UUBotWindSubsystem>();
    if (!TestNotNull(TEXT("Wind subsystem in a game world"), Wind))
    {
        return false;
    }
    TestTrue(TEXT("Calm by default"), Wind->GetWindVelocityAt(FVector::ZeroVector).IsZero());

    AUBotWind* First = SpawnWind(Scope, MakeWind(FVector(2.0, 0.0, 0.0)));
    SpawnWind(Scope, MakeWind(FVector(0.0, -1.0, 0.0)));
    TestTrue(TEXT("Actors add up"), Wind->GetWindVelocityAt(FVector::ZeroVector).Equals(FVector(2.0, -1.0, 0.0), 1e-12));
    Wind->SetScriptedWind(MakeWind(FVector(0.0, 0.0, 0.5)));
    TestTrue(TEXT("Scripted wind adds too"), Wind->GetWindVelocityAt(FVector(1000.0, 0.0, 0.0)).Equals(FVector(2.0, -1.0, 0.5), 1e-12));
    First->SetWindSettings(MakeWind(FVector(4.0, 0.0, 0.0)));
    TestTrue(TEXT("Changed settings apply"), Wind->GetWindVelocityAt(FVector::ZeroVector).Equals(FVector(4.0, -1.0, 0.5), 1e-12));
    First->SetWindEnabled(false);
    Wind->SetScriptedWind(FUBotWindSettings(), false);
    TestTrue(TEXT("Disabled and cleared"), Wind->GetWindVelocityAt(FVector::ZeroVector).Equals(FVector(0.0, -1.0, 0.0), 1e-12));
    First->RouteEndPlay(EEndPlayReason::Destroyed);
    TestTrue(TEXT("Ended actors leave"), Wind->GetWindVelocityAt(FVector::ZeroVector).Equals(FVector(0.0, -1.0, 0.0), 1e-12));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUBotEnvironmentWindCoreQueryTest, "UBotEnvironment.Wind.CoreQuery", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUBotEnvironmentWindCoreQueryTest::RunTest(const FString& Parameters)
{
    using namespace UBotEnvironmentTests;
    FScopedEnvironmentTestWorld Scope;
    UUBotWindSubsystem* Wind = Scope.Get()->GetSubsystem<UUBotWindSubsystem>();
    UUBotWindQuerySubsystem* Query = Scope.Get()->GetSubsystem<UUBotWindQuerySubsystem>();
    if (!TestNotNull(TEXT("Environment wind"), Wind) || !TestNotNull(TEXT("Core query"), Query))
    {
        return false;
    }
    TestTrue(TEXT("Core route is calm initially"), Query->GetWindVelocity(FVector::ZeroVector, 0.0).IsZero());

    AUBotWind* Actor = SpawnWind(Scope, MakeWind(FVector(2.0, -1.0, 0.5), FVector(0.5, 0.2, 0.1), 42));
    Wind->SetScriptedWind(MakeWind(FVector(-0.2, 0.3, 0.0), FVector(0.1, 0.0, 0.2), 17));
    for (const double Time : {0.0, 0.125, 7.0, 37.2})
    {
        const FVector Expected = Wind->GetWindVelocity(FVector::ZeroVector, Time);
        TestTrue(TEXT("Provider automatically supplies the Core query"), Query->GetWindVelocity(FVector::ZeroVector, Time) == Expected);
        TestTrue(TEXT("Uniform wind at another location"), Query->GetWindVelocity(FVector(12000.0, -8000.0, 15000.0), Time) == Expected);
    }

    Wind->RegisterWindSource(Actor);
    const FVector Once = Wind->GetWindVelocity(FVector::ZeroVector, 2.0);
    Wind->RegisterWindSource(Actor);
    TestTrue(TEXT("A source registers once"), Query->GetWindVelocity(FVector::ZeroVector, 2.0) == Once);
    Actor->SetWindSettings(MakeWind(FVector(4.0, 0.0, 0.0)));
    Wind->SetScriptedWind(FUBotWindSettings(), false);
    TestTrue(TEXT("Core route observes changed actor settings"), Query->GetWindVelocity(FVector::ZeroVector, 2.0).Equals(FVector(4.0, 0.0, 0.0), 1e-12));
    Actor->Destroy();
    TestTrue(TEXT("Destroyed actor is removed from the Core route"), Query->GetWindVelocity(FVector::ZeroVector, 2.0).IsZero());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUBotEnvironmentWindWorldIsolationTest, "UBotEnvironment.Wind.WorldIsolation", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUBotEnvironmentWindWorldIsolationTest::RunTest(const FString& Parameters)
{
    using namespace UBotEnvironmentTests;
    FScopedEnvironmentTestWorld First;
    UUBotWindSubsystem* FirstWind = First.Get()->GetSubsystem<UUBotWindSubsystem>();
    UUBotWindQuerySubsystem* FirstQuery = First.Get()->GetSubsystem<UUBotWindQuerySubsystem>();
    if (!TestNotNull(TEXT("First wind"), FirstWind) || !TestNotNull(TEXT("First query"), FirstQuery))
    {
        return false;
    }
    FirstWind->SetScriptedWind(MakeWind(FVector(1.0, 2.0, 3.0)));
    {
        FScopedEnvironmentTestWorld Second;
        UUBotWindSubsystem* SecondWind = Second.Get()->GetSubsystem<UUBotWindSubsystem>();
        UUBotWindQuerySubsystem* SecondQuery = Second.Get()->GetSubsystem<UUBotWindQuerySubsystem>();
        if (!TestNotNull(TEXT("Second wind"), SecondWind) || !TestNotNull(TEXT("Second query"), SecondQuery))
        {
            return false;
        }
        TestTrue(TEXT("Another world starts calm"), SecondQuery->GetWindVelocity(FVector::ZeroVector, 5.0).IsZero());
        SpawnWind(Second, MakeWind(FVector(-4.0, 0.0, 0.0)));
        SecondWind->SetScriptedWind(MakeWind(FVector(0.0, 5.0, 0.0)));
        TestTrue(TEXT("Second world uses its own sources"), SecondQuery->GetWindVelocity(FVector::ZeroVector, 5.0).Equals(FVector(-4.0, 5.0, 0.0), 1e-12));
        TestTrue(TEXT("First world keeps its sources"), FirstQuery->GetWindVelocity(FVector::ZeroVector, 5.0).Equals(FVector(1.0, 2.0, 3.0), 1e-12));
    }
    TestTrue(TEXT("Destroying another world leaves the first intact"), FirstQuery->GetWindVelocity(FVector::ZeroVector, 5.0).Equals(FVector(1.0, 2.0, 3.0), 1e-12));
    {
        FScopedEnvironmentTestWorld Replacement;
        UUBotWindQuerySubsystem* ReplacementQuery = Replacement.Get()->GetSubsystem<UUBotWindQuerySubsystem>();
        if (!TestNotNull(TEXT("Replacement query"), ReplacementQuery))
        {
            return false;
        }
        TestTrue(TEXT("A new world inherits no old sources"), ReplacementQuery->GetWindVelocity(FVector::ZeroVector, 5.0).IsZero());
    }
    return true;
}

#endif
