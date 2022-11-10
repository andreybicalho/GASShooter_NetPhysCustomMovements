// Copyright 2022 Andrey Bicalho.


#include "Characters/Abilities/AbilityTasks/GSAT_ApplyPhysCustomJetPack.h"
#include "Characters/GSCharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"

UGSAT_ApplyPhysCustomJetPack::UGSAT_ApplyPhysCustomJetPack(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UGSAT_ApplyPhysCustomJetPack* UGSAT_ApplyPhysCustomJetPack::PhysJetPack(UGameplayAbility* OwningAbility, FName TaskInstanceName, const FVector& inJetPackAcceleration, float inMaxSpeed)
{
	UGSAT_ApplyPhysCustomJetPack* abilityTask = NewAbilityTask<UGSAT_ApplyPhysCustomJetPack>(OwningAbility, TaskInstanceName);

	if (abilityTask)
	{
		abilityTask->CustomMovementName = TaskInstanceName;
		abilityTask->MaxSpeed = inMaxSpeed;
		abilityTask->JetPackAcceleration = inJetPackAcceleration;
		abilityTask->InitAndApply();

		return abilityTask;
	}

	return nullptr;
}

void UGSAT_ApplyPhysCustomJetPack::InitAndApply()
{
	if (AbilitySystemComponent->AbilityActorInfo->MovementComponent.IsValid())
	{
		CharacterMovementComponent = Cast<UGSCharacterMovementComponent>(AbilitySystemComponent->AbilityActorInfo->MovementComponent.Get());

		if (CharacterMovementComponent)
		{
			TSharedPtr<FPhysCustomMovement_JetPack> movement = MakeShared<FPhysCustomMovement_JetPack>();

			movement->MovementName = CustomMovementName;
			movement->CharacterMovementComponent = CharacterMovementComponent;
			movement->MaxSpeed = MaxSpeed;
			movement->BaseAcceleration = JetPackAcceleration;
			movement->OnCustomMovementEnd.AddDynamic(this, &ThisClass::OnPhysCustomMovementEnded);

			Move(PhysCustomMovement, movement);
			CharacterMovementComponent->StartPhysCustomMovement(PhysCustomMovement);
		}
	}
}
