// Copyright 2022 Andrey Bicalho.


#include "Characters/Abilities/AbilityTasks/GSAT_ApplyPhysCustomSprint.h"
#include "Characters/GSCharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"

UGSAT_ApplyPhysCustomSprint::UGSAT_ApplyPhysCustomSprint(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UGSAT_ApplyPhysCustomSprint* UGSAT_ApplyPhysCustomSprint::PhysSprint(UGameplayAbility* OwningAbility, FName TaskInstanceName, float inMaxSpeed)
{
	UGSAT_ApplyPhysCustomSprint* abilityTask = NewAbilityTask<UGSAT_ApplyPhysCustomSprint>(OwningAbility, TaskInstanceName);

	if (abilityTask)
	{
		abilityTask->CustomMovementName = TaskInstanceName;
		abilityTask->MaxSpeed = inMaxSpeed;
		abilityTask->InitAndApply();

		return abilityTask;
	}

	return nullptr;
}

void UGSAT_ApplyPhysCustomSprint::InitAndApply()
{
	if (AbilitySystemComponent->AbilityActorInfo->MovementComponent.IsValid())
	{
		CharacterMovementComponent = Cast<UGSCharacterMovementComponent>(AbilitySystemComponent->AbilityActorInfo->MovementComponent.Get());

		if (CharacterMovementComponent)
		{
			TSharedPtr<FPhysCustomMovement_Sprint> movement = MakeShared<FPhysCustomMovement_Sprint>();

			movement->MovementName = CustomMovementName;
			movement->CharacterMovementComponent = CharacterMovementComponent;
			movement->MaxSpeed = MaxSpeed;
			movement->OnCustomMovementEnd.AddDynamic(this, &ThisClass::OnPhysCustomMovementEnded);

			Move(PhysCustomMovement, movement);
			CharacterMovementComponent->StartPhysCustomMovement(PhysCustomMovement);
		}
	}
}

