// Copyright 2022 Andrey Bicalho.

#include "Abilities/Tasks/GAT_ApplyPhysCustomMovement_Sprint.h"
#include "Components/PMCharacterMovementComponent.h"
#include "Movements/PhysCustomMovement_Sprint.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"

UGAT_ApplyPhysCustomMovement_Sprint::UGAT_ApplyPhysCustomMovement_Sprint(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UGAT_ApplyPhysCustomMovement_Sprint* UGAT_ApplyPhysCustomMovement_Sprint::PhysSprint(UGameplayAbility* OwningAbility, FName TaskInstanceName, float inMaxSpeed)
{
	UGAT_ApplyPhysCustomMovement_Sprint* abilityTask = NewAbilityTask<UGAT_ApplyPhysCustomMovement_Sprint>(OwningAbility, TaskInstanceName);

	if (abilityTask)
	{
		abilityTask->CustomMovementName = TaskInstanceName;
		abilityTask->MaxSpeed = inMaxSpeed;
		abilityTask->InitAndApply();

		return abilityTask;
	}

	return nullptr;
}

void UGAT_ApplyPhysCustomMovement_Sprint::InitAndApply()
{
	if (AbilitySystemComponent->AbilityActorInfo->MovementComponent.IsValid())
	{
		CharacterMovementComponent = Cast<UPMCharacterMovementComponent>(AbilitySystemComponent->AbilityActorInfo->MovementComponent.Get());

		if (CharacterMovementComponent.IsValid())
		{
			TSharedPtr<FPhysCustomMovement_Sprint> movement = MakeShared<FPhysCustomMovement_Sprint>();

			movement->MovementName = CustomMovementName;
			movement->CharacterMovementComponent = CharacterMovementComponent;
			movement->MaxSpeed = MaxSpeed;
			movement->OnCustomMovementEnd.AddDynamic(this, &ThisClass::OnPhysCustomMovementEnded);

			Move(PhysCustomMovement, movement);
			bActivated = CharacterMovementComponent->StartPhysCustomMovement(PhysCustomMovement);
		}
	}
}
