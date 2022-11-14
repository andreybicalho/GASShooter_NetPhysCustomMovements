// Copyright 2022 Andrey Bicalho.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "Characters/PhysCustomMovement.h"
#include "GSAT_ApplyPhysCustomMovementBase.generated.h"

class UGSCharacterMovementComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FApplyPhysCustomMovementDelegate);

/**
 * Base class for ability tasks that apply Physics Custom Movements, providing basic functionality to manage the custom movement.
 */
UCLASS()
class GASSHOOTER_API UGSAT_ApplyPhysCustomMovementBase : public UAbilityTask
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FApplyPhysCustomMovementDelegate OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FApplyPhysCustomMovementDelegate OnFinish;
	
	UPROPERTY(BlueprintAssignable)
	FApplyPhysCustomMovementDelegate OnFailed;

protected:
	UPROPERTY()
	UGSCharacterMovementComponent* CharacterMovementComponent = nullptr;

	FName CustomMovementName;

	float MaxSpeed = 2022.f;

	TSharedPtr<FPhysCustomMovement> PhysCustomMovement;

	bool bActivated = false;

public:
	UGSAT_ApplyPhysCustomMovementBase(const FObjectInitializer& ObjectInitializer);

	virtual void OnDestroy(bool AbilityIsEnding) override;

	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks|PhysCustomMovement")
	virtual void Finish();

protected:
	virtual void InitAndApply() {};

	UFUNCTION()
	virtual void OnPhysCustomMovementEnded();

	virtual void Activate() override;
};
