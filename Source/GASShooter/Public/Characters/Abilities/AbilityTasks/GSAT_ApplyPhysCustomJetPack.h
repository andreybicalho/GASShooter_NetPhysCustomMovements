// Copyright 2022 Andrey Bicalho.

#pragma once

#include "CoreMinimal.h"
#include "Characters/Abilities/AbilityTasks/GSAT_ApplyPhysCustomMovementBase.h"
#include "Characters/PhysCustomMovement.h"
#include "GSAT_ApplyPhysCustomJetPack.generated.h"

/**
 * 
 */
UCLASS()
class GASSHOOTER_API UGSAT_ApplyPhysCustomJetPack : public UGSAT_ApplyPhysCustomMovementBase
{
	GENERATED_BODY()

	FVector JetPackAcceleration = FVector::ZeroVector;

public:
	UGSAT_ApplyPhysCustomJetPack(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UGSAT_ApplyPhysCustomJetPack* PhysJetPack(UGameplayAbility* OwningAbility, FName TaskInstanceName, const FVector& inJetPackAcceleration, float inMaxSpeed);

protected:
	virtual void InitAndApply() override;
};
