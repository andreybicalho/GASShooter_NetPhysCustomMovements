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
		if (FPhysCustomMovement_NonDeterministicMove*  movement = static_cast<FPhysCustomMovement_NonDeterministicMove*>(PhysCustomMovement.Get()))
		{
			movement->OnReachedTime.RemoveAll(this);
		}
	}

	Super::OnDestroy(AbilityIsEnding);
}

UGAT_ApplyPhysCustomMovement_NonDeterministicMove* UGAT_ApplyPhysCustomMovement_NonDeterministicMove::PhysNonDeterministicMove(UGameplayAbility* OwningAbility, FName TaskInstanceName, float inInitialTimeToWait, const float inMaxSpeed, const float inMaxAcceleration, const float MaxBrakingDeceleration)
{
	UGAT_ApplyPhysCustomMovement_NonDeterministicMove* abilityTask = NewAbilityTask<UGAT_ApplyPhysCustomMovement_NonDeterministicMove>(OwningAbility, TaskInstanceName);

	if (abilityTask)
	{
		abilityTask->CustomMovementName = TaskInstanceName;
		abilityTask->MaxSpeed = inMaxSpeed;
		abilityTask->MaxAcceleration = inMaxAcceleration;
		abilityTask->MaxBrakingDeceleration = MaxBrakingDeceleration;
        abilityTask->InitialTimeToWait = inInitialTimeToWait;
		abilityTask->InitAndApply();

		return abilityTask;
	}

	return nullptr;
}

void UGAT_ApplyPhysCustomMovement_NonDeterministicMove::SetWaitTime(const float waitTime)
{
	if (FPhysCustomMovement_NonDeterministicMove* movement = static_cast<FPhysCustomMovement_NonDeterministicMove*>(PhysCustomMovement.Get()))
	{
		movement->TimeToWait = waitTime;
		movement->ElapsedTime = 0.f;

		UE_LOG(LogPhysCustomMovement, Display, TEXT("%s: %s: TimeToWait = %.2f | ElapsedTime = %.2f"),
			ANSI_TO_TCHAR(__FUNCTION__),
			*UEnum::GetValueAsString(GetAvatarActor()->GetLocalRole()),
			movement->TimeToWait,
			movement->ElapsedTime
		);
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
			movement->MaxAcceleration = MaxAcceleration;
			movement->MaxBrakingDeceleration = MaxBrakingDeceleration;
			movement->TimeToWait = InitialTimeToWait;
			movement->OnCustomMovementEnd.AddDynamic(this, &ThisClass::OnPhysCustomMovementEnded);
			movement->OnReachedTime.AddDynamic(this, &ThisClass::OnPhysCustomMovementReachedTime);
			
			// NOTE: bind predicted properties from the movement data members that carry information about the movement so the server can adjust accordingly 
			movement->BindFloatProperty("TimeToWait");
			movement->BindFloatProperty("MovementDirectionSign");
			movement->BindFloatProperty("ElapsedTime");

			PhysCustomMovement = movement;
			bActivated = CharacterMovementComponent->StartPhysCustomMovement(PhysCustomMovement);
		}
	}
}
