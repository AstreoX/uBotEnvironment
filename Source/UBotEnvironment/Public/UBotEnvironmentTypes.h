#pragma once

#include "CoreMinimal.h"
#include "UBotEnvironmentTypes.generated.h"

/** Wind: a constant part plus gusts. The gusts are a sum of sines with seeded phases, so the same settings give the same wind at the same sim time. */
USTRUCT(BlueprintType)
struct UBOTENVIRONMENT_API FUBotWindSettings
{
    GENERATED_BODY()

    /** Steady wind velocity in the world frame, m/s (the direction the air moves). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind")
    FVector ConstantVelocityMetersPerSecond = FVector::ZeroVector;

    /** Standard deviation of the gusts per world axis, m/s. Zero turns gusts off. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind")
    FVector GustStandardDeviationMetersPerSecond = FVector::ZeroVector;

    /** Typical duration of a gust, s. Longer is smoother. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind", meta = (ClampMin = "0.05", Units = "Seconds"))
    float GustCorrelationTimeSeconds = 2.0f;

    /** Seed of the gust pattern. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind")
    int32 Seed = 0;
};
