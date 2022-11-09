// Copyright 2022 Andrey Bicalho.

#pragma once

#include "CoreMinimal.h"
#include "Characters/Abilities/AbilityTasks/GSAT_ApplyPhysCustomMovementBase.h"
#include "Characters/PhysCustomMovement.h"
#include "GSAT_ApplyPhysCustomSprint.generated.h"

USTRUCT()
struct GASSHOOTER_API FPhysCustomMovement_Sprint : public FPhysCustomMovement
{
	GENERATED_USTRUCT_BODY()

	FPhysCustomMovement_Sprint()
	{
	};

	virtual ~FPhysCustomMovement_Sprint() {}

	virtual void UpdateMovement(const float deltaTime, const FVector& oldVelocity, FVector& outVelocity) override
	{
		CurrentTime += deltaTime;
		
		outVelocity = oldVelocity * GetMaxSpeed();
		outVelocity = outVelocity.GetClampedToMaxSize(GetMaxSpeed());
	};
};

/**
 * 
 */
UCLASS()
class GASSHOOTER_API UGSAT_ApplyPhysCustomSprint : public UGSAT_ApplyPhysCustomMovementBase
{
	GENERATED_BODY()
	
	FPhysCustomMovement_Sprint PhysSprintMovement;

public:
	UGSAT_ApplyPhysCustomSprint(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UGSAT_ApplyPhysCustomSprint* PhysSprint(UGameplayAbility* OwningAbility, FName TaskInstanceName, float inMaxSpeed);

	virtual void Activate() override;

	virtual void OnDestroy(bool AbilityIsEnding) override;

	virtual void Finish() override;

protected:
	virtual void InitAndApply() override;

	UFUNCTION()
		void OnPhysSprintEnded();
};
