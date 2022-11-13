// Copyright 2022 Andrey Bicalho.

#pragma once

#include "CoreMinimal.h"
#include "Characters/Abilities/AbilityTasks/GSAT_ApplyPhysCustomMovementBase.h"
#include "GSAT_ApplyPhysCustomSprint.generated.h"

/**
 * 
 */
UCLASS()
class GASSHOOTER_API UGSAT_ApplyPhysCustomSprint : public UGSAT_ApplyPhysCustomMovementBase
{
	GENERATED_BODY()

public:
	UGSAT_ApplyPhysCustomSprint(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UGSAT_ApplyPhysCustomSprint* PhysSprint(UGameplayAbility* OwningAbility, FName TaskInstanceName, float inMaxSpeed);

protected:
	virtual void InitAndApply() override;
};
