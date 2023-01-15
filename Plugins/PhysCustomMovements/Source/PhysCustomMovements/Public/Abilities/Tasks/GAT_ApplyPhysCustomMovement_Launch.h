// Copyright 2022 Andrey Bicalho.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/GAT_ApplyPhysCustomMovementBase.h"
#include "GAT_ApplyPhysCustomMovement_Launch.generated.h"

/**
 * 
 */
UCLASS()
class PHYSCUSTOMMOVEMENTS_API UGAT_ApplyPhysCustomMovement_Launch : public UGAT_ApplyPhysCustomMovementBase
{
	GENERATED_BODY()
		
	FVector LaunchVelocity = FVector::ZeroVector;

	bool bXYOverride = false;

	bool bZOverride = false;

public:
	UGAT_ApplyPhysCustomMovement_Launch(const FObjectInitializer& ObjectInitializer);

	/** Launch Character with velocity. */
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UGAT_ApplyPhysCustomMovement_Launch* PhysLaunch(UGameplayAbility* OwningAbility, FName TaskInstanceName,
												const FVector& inLaunchVelocity, bool bInXYOverride, bool bInZOverride, float inMaxSpeed);

protected:
	virtual void InitAndApply() override;
};
