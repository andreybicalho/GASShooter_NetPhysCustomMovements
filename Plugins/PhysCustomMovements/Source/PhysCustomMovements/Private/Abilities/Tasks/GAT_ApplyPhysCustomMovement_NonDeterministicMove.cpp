// Copyright 2022 Andrey Bicalho.

#include "Abilities/Tasks/GAT_ApplyPhysCustomMovement_NonDeterministicMove.h"
#include "Components/PMCharacterMovementComponent.h"
#include "Movements/PhysCustomMovement_NonDeterministicMove.h"
#include "AbilitySystemComponent.h"

UGAT_ApplyPhysCustomMovement_NonDeterministicMove::UGAT_ApplyPhysCustomMovement_NonDeterministicMove(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UGAT_ApplyPhysCustomMovement_NonDeterministicMove::OnDestroy(bool AbilityIsEnding)
{
	if (PhysCustomMovement.IsValid())
	{
		if (const auto movement = static_cast<FPhysCustomMovement_NonDeterministicMove*>(PhysCustomMovement.Get()))
		{
			movement->OnReachedTime.RemoveAll(this);
		}
	}

	Super::OnDestroy(AbilityIsEnding);
}

UGAT_ApplyPhysCustomMovement_NonDeterministicMove* UGAT_ApplyPhysCustomMovement_NonDeterministicMove::PhysNonDeterministicMove(UGameplayAbility* OwningAbility, FName TaskInstanceName, float inInitialTimeToWait, float inMaxSpeed)
{
	UGAT_ApplyPhysCustomMovement_NonDeterministicMove* abilityTask = NewAbilityTask<UGAT_ApplyPhysCustomMovement_NonDeterministicMove>(OwningAbility, TaskInstanceName);

	if (abilityTask)
	{
		abilityTask->CustomMovementName = TaskInstanceName;
		abilityTask->MaxSpeed = inMaxSpeed;
        abilityTask->InitialTimeToWait = inInitialTimeToWait;
		abilityTask->InitAndApply();

		return abilityTask;
	}

	return nullptr;
}

void UGAT_ApplyPhysCustomMovement_NonDeterministicMove::SetWaitTime(const float waitTime)
{
	if (const auto movement = static_cast<FPhysCustomMovement_NonDeterministicMove*>(PhysCustomMovement.Get()))
	{
		movement->TimeToWait = waitTime;
		movement->ElapsedTime = 0.f;
	}
}

void UGAT_ApplyPhysCustomMovement_NonDeterministicMove::OnPhysCustomMovementReachedTime()
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnReachedTime.Broadcast();
	}
}

void UGAT_ApplyPhysCustomMovement_NonDeterministicMove::InitAndApply()
{
	if (AbilitySystemComponent->AbilityActorInfo->MovementComponent.IsValid())
	{
		CharacterMovementComponent = Cast<UPMCharacterMovementComponent>(AbilitySystemComponent->AbilityActorInfo->MovementComponent.Get());

		if (CharacterMovementComponent.IsValid())
		{
			TSharedPtr<FPhysCustomMovement_NonDeterministicMove> movement = MakeShared<FPhysCustomMovement_NonDeterministicMove>();

			movement->MovementName = CustomMovementName;
			movement->CharacterMovementComponent = CharacterMovementComponent;
			movement->MaxSpeed = MaxSpeed;
			movement->TimeToWait = InitialTimeToWait;
			movement->OnCustomMovementEnd.AddDynamic(this, &ThisClass::OnPhysCustomMovementEnded);
			movement->OnReachedTime.AddDynamic(this, &ThisClass::OnPhysCustomMovementReachedTime);

			Move(PhysCustomMovement, movement);
			bActivated = CharacterMovementComponent->StartPhysCustomMovement(PhysCustomMovement);
		}
	}
}
