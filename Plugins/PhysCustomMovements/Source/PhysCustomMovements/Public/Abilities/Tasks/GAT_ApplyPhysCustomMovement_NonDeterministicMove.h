// Copyright 2022 Andrey Bicalho.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/GAT_ApplyPhysCustomMovementBase.h"
#include "GAT_ApplyPhysCustomMovement_NonDeterministicMove.generated.h"

/**
* An example of a non deterministic movement in which we have variables that can't be predicted (TimeToWait, MovementDirectionSign and ElapsedTime).
* During the simulation server and autonomous proxy will end up with different values and thus be out of sync.
* To fix that we must replicate those variables from client to server and apply them correctly so they still be in sync.
*
* Check FPMSavedMove, FPMCharacterNetworkMoveData, UPMCharacterMovementComponent and specially UPMCharacterMovementComponent::MoveAutonomous to see how it works.
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
