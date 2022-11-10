// Copyright 2022 Andrey Bicalho.

#pragma once

#include "CoreMinimal.h"
#include "Characters/Abilities/AbilityTasks/GSAT_ApplyPhysCustomMovementBase.h"
#include "Characters/PhysCustomMovement.h"
#include "GSAT_ApplyPhysCustomJump.generated.h"

/**
 * 
 */
UCLASS()
class GASSHOOTER_API UGSAT_ApplyPhysCustomJump : public UGSAT_ApplyPhysCustomMovementBase
{
	GENERATED_BODY()
		
	FVector LaunchVelocity = FVector::ZeroVector;

	bool bXYOverride = false;

	bool bZOverride = false;

public:
	UGSAT_ApplyPhysCustomJump(const FObjectInitializer& ObjectInitializer);

	/** Apply jump to character's movement */
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UGSAT_ApplyPhysCustomJump* PhysJump(UGameplayAbility* OwningAbility, FName TaskInstanceName,
												const FVector& inLaunchVelocity, bool bInXYOverride, bool bInZOverride, float inMaxSpeed);

protected:
	virtual void InitAndApply() override;
};
