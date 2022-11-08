// Copyright 2022 Andrey Bicalho.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "GSAT_ApplyPhysCustomMovementBase.generated.h"

class UGSCharacterMovementComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FApplyPhysCustomMovementDelegate);

/**
 * Base class for ability tasks that apply Physics Custom Movements.
 */
UCLASS()
class GASSHOOTER_API UGSAT_ApplyPhysCustomMovementBase : public UAbilityTask
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FApplyPhysCustomMovementDelegate OnFinish;

protected:
	UPROPERTY()
	UGSCharacterMovementComponent* CharacterMovementComponent = nullptr;

	UPROPERTY(Replicated)
	FName CustomMovementName;

public:
	UGSAT_ApplyPhysCustomMovementBase(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks|PhysCustomMovement")
	virtual void Finish() {};

	virtual void InitSimulatedTask(UGameplayTasksComponent& InGameplayTasksComponent) override;

protected:
	virtual void InitAndApply() {};
};
