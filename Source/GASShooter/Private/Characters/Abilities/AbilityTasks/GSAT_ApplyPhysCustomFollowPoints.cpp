// Copyright 2022 Andrey Bicalho.


#include "Characters/Abilities/AbilityTasks/GSAT_ApplyPhysCustomFollowPoints.h"
#include "Characters/GSCharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"

UGSAT_ApplyPhysCustomFollowPoints::UGSAT_ApplyPhysCustomFollowPoints(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UGSAT_ApplyPhysCustomFollowPoints* UGSAT_ApplyPhysCustomFollowPoints::PhysFollowPoints(UGameplayAbility* OwningAbility, FName TaskInstanceName, const TArray<FVector>& inPathPoints, float inConsumePointDistanceThreshold, float inMaxSpeed)
{
	UGSAT_ApplyPhysCustomFollowPoints* abilityTask = NewAbilityTask<UGSAT_ApplyPhysCustomFollowPoints>(OwningAbility, TaskInstanceName);

	if (abilityTask)
	{
		abilityTask->CustomMovementName = TaskInstanceName;
		abilityTask->MaxSpeed = inMaxSpeed;
		abilityTask->PathPoints = inPathPoints;
		abilityTask->ConsumePointDistanceThreshold = inConsumePointDistanceThreshold;

		abilityTask->InitAndApply();

		return abilityTask;
	}

	return nullptr;
}

void UGSAT_ApplyPhysCustomFollowPoints::InitAndApply()
{
	if (AbilitySystemComponent->AbilityActorInfo->MovementComponent.IsValid())
	{
		CharacterMovementComponent = Cast<UGSCharacterMovementComponent>(AbilitySystemComponent->AbilityActorInfo->MovementComponent.Get());

		if (CharacterMovementComponent)
		{
			TSharedPtr<FPhysCustomMovement_FollowPoints> movement = MakeShared<FPhysCustomMovement_FollowPoints>();

			movement->MovementName = CustomMovementName;
			movement->MaxSpeed = MaxSpeed;
			movement->CharacterMovementComponent = CharacterMovementComponent;
			movement->PathPoints = PathPoints;
			movement->ConsumePointDistanceThreshold = ConsumePointDistanceThreshold;
			movement->OnCustomMovementEnd.AddDynamic(this, &ThisClass::OnPhysCustomMovementEnded);

			Move(PhysCustomMovement, movement);
			bActivated = CharacterMovementComponent->StartPhysCustomMovement(PhysCustomMovement);
		}
	}
}
