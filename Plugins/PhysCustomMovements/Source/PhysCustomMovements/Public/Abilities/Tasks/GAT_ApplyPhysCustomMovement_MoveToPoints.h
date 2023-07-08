// Copyright 2022 Andrey Bicalho.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/GAT_ApplyPhysCustomMovementBase.h"
#include "GAT_ApplyPhysCustomMovement_MoveToPoints.generated.h"

/**
 * 
 */
UCLASS()
class PHYSCUSTOMMOVEMENTS_API UGAT_ApplyPhysCustomMovement_MoveToPoints : public UGAT_ApplyPhysCustomMovementBase
{
	GENERATED_BODY()
	
	TArray<FVector> PathPoints = {};

	float ConsumePointDistanceThreshold = 50.f;

public:
	UGAT_ApplyPhysCustomMovement_MoveToPoints(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UGAT_ApplyPhysCustomMovement_MoveToPoints* PhysMoveToPoints(UGameplayAbility* OwningAbility, FName TaskInstanceName,
			const TArray<FVector>& inPathPoints, float inConsumePointDistanceThreshold, 
			const float inMaxSpeed, const float inMaxAcceleration, const float MaxBrakingDeceleration);

protected:
	virtual void InitAndApply() override;
};
