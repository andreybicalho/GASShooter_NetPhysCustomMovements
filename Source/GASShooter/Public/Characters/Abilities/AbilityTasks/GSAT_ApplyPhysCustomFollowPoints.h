// Copyright 2022 Andrey Bicalho.

#pragma once

#include "CoreMinimal.h"
#include "Characters/Abilities/AbilityTasks/GSAT_ApplyPhysCustomMovementBase.h"
#include "GSAT_ApplyPhysCustomFollowPoints.generated.h"

/**
 * 
 */
UCLASS()
class GASSHOOTER_API UGSAT_ApplyPhysCustomFollowPoints : public UGSAT_ApplyPhysCustomMovementBase
{
	GENERATED_BODY()
	

	TArray<FVector> PathPoints = {};

	float ConsumePointDistanceThreshold = 50.f;

public:
	UGSAT_ApplyPhysCustomFollowPoints(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UGSAT_ApplyPhysCustomFollowPoints* PhysFollowPoints(UGameplayAbility* OwningAbility, FName TaskInstanceName,
			const TArray<FVector>& inPathPoints, float inConsumePointDistanceThreshold = 100.f, float inMaxSpeed = 900);

protected:
	virtual void InitAndApply() override;
};
