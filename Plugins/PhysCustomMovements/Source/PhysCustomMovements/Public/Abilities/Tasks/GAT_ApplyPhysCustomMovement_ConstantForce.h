// Copyright 2022 Andrey Bicalho.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/GAT_ApplyPhysCustomMovementBase.h"
#include "GAT_ApplyPhysCustomMovement_ConstantForce.generated.h"

/**
 * 
 */
UCLASS()
class PHYSCUSTOMMOVEMENTS_API UGAT_ApplyPhysCustomMovement_ConstantForce : public UGAT_ApplyPhysCustomMovementBase
{
	GENERATED_BODY()

	FVector Force = FVector::ZeroVector;

public:
	UGAT_ApplyPhysCustomMovement_ConstantForce(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UGAT_ApplyPhysCustomMovement_ConstantForce* PhysConstantForce(UGameplayAbility* OwningAbility, FName TaskInstanceName, const FVector& inForce, float inMaxSpeed);

protected:
	virtual void InitAndApply() override;
};
