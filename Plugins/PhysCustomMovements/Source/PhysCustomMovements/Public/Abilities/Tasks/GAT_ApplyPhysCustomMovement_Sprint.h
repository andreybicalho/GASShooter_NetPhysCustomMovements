// Copyright 2022 Andrey Bicalho.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/GAT_ApplyPhysCustomMovementBase.h"
#include "GAT_ApplyPhysCustomMovement_Sprint.generated.h"

/**
 * 
 */
UCLASS()
class PHYSCUSTOMMOVEMENTS_API UGAT_ApplyPhysCustomMovement_Sprint : public UGAT_ApplyPhysCustomMovementBase
{
	GENERATED_BODY()

public:
	UGAT_ApplyPhysCustomMovement_Sprint(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UGAT_ApplyPhysCustomMovement_Sprint* PhysSprint(UGameplayAbility* OwningAbility, FName TaskInstanceName, float inMaxSpeed);

protected:
	virtual void InitAndApply() override;
};
