// Copyright 2022 Andrey Bicalho.

#include "Abilities/Tasks/GAT_ApplyPhysCustomMovement_MoveToPoints.h"
#include "Components/PMCharacterMovementComponent.h"
#include "Movements/PhysCustomMovement_MoveToPoints.h"
#include "AbilitySystemComponent.h"

UGAT_ApplyPhysCustomMovement_MoveToPoints::UGAT_ApplyPhysCustomMovement_MoveToPoints(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UGAT_ApplyPhysCustomMovement_MoveToPoints* UGAT_ApplyPhysCustomMovement_MoveToPoints::PhysMoveToPoints(UGameplayAbility* OwningAbility, FName TaskInstanceName, const TArray<FVector>& inPathPoints, float inConsumePointDistanceThreshold, float inMaxSpeed)
{
	UGAT_ApplyPhysCustomMovement_MoveToPoints* abilityTask = NewAbilityTask<UGAT_ApplyPhysCustomMovement_MoveToPoints>(OwningAbility, TaskInstanceName);

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

void UGAT_ApplyPhysCustomMovement_MoveToPoints::InitAndApply()
{
	if (AbilitySystemComponent->AbilityActorInfo->MovementComponent.IsValid())
	{
		CharacterMovementComponent = Cast<UPMCharacterMovementComponent>(AbilitySystemComponent->AbilityActorInfo->MovementComponent.Get());

		if (CharacterMovementComponent.IsValid())
		{
			TSharedPtr<FPhysCustomMovement_MoveToPoints> movement = MakeShared<FPhysCustomMovement_MoveToPoints>();

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
