// Copyright 2022 Andrey Bicalho.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/GAT_ApplyPhysCustomMovementBase.h"
#include "GAT_ApplyPhysCustomMovement_NonDeterministicMove.generated.h"

/**
 * 
 */
UCLASS()
class PHYSCUSTOMMOVEMENTS_API UGAT_ApplyPhysCustomMovement_NonDeterministicMove : public UGAT_ApplyPhysCustomMovementBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FApplyPhysCustomMovementDelegate OnReachedTime;

    float InitialTimeToWait = 0.f;

public:
	UGAT_ApplyPhysCustomMovement_NonDeterministicMove(const FObjectInitializer& ObjectInitializer);

	virtual void OnDestroy(bool AbilityIsEnding) override;

	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UGAT_ApplyPhysCustomMovement_NonDeterministicMove* PhysNonDeterministicMove(UGameplayAbility* OwningAbility, FName TaskInstanceName, float inInitialTimeToWait, float inMaxSpeed);

	UFUNCTION(BlueprintCallable)
	void SetWaitTime(const float waitTime);

	UFUNCTION()
	void OnPhysCustomMovementReachedTime();

protected:
	virtual void InitAndApply() override;
};
