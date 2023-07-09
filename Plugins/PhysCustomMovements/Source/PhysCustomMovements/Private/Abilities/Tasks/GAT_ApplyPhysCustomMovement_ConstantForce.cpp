// Copyright 2022 Andrey Bicalho.


#include "Abilities/Tasks/GAT_ApplyPhysCustomMovement_ConstantForce.h"
#include "Components/PMCharacterMovementComponent.h"
#include "Movements/PhysCustomMovement_ConstantForce.h"
#include "AbilitySystemComponent.h"

UGAT_ApplyPhysCustomMovement_ConstantForce::UGAT_ApplyPhysCustomMovement_ConstantForce(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UGAT_ApplyPhysCustomMovement_ConstantForce* UGAT_ApplyPhysCustomMovement_ConstantForce::PhysConstantForce(UGameplayAbility* OwningAbility, FName TaskInstanceName, const FVector& inForce, const float inMaxSpeed, const float inMaxAcceleration, const float MaxBrakingDeceleration)
{
	UGAT_ApplyPhysCustomMovement_ConstantForce* abilityTask = NewAbilityTask<UGAT_ApplyPhysCustomMovement_ConstantForce>(OwningAbility, TaskInstanceName);

	if (abilityTask)
	{
		abilityTask->CustomMovementName = TaskInstanceName;
		abilityTask->MaxSpeed = inMaxSpeed;
		abilityTask->MaxAcceleration = inMaxAcceleration;
		abilityTask->MaxBrakingDeceleration = MaxBrakingDeceleration;
		abilityTask->Force = inForce;
		abilityTask->InitAndApply();

		return abilityTask;
	}

	return nullptr;
}

void UGAT_ApplyPhysCustomMovement_ConstantForce::InitAndApply()
{
	if (AbilitySystemComponent->AbilityActorInfo->MovementComponent.IsValid())
	{
		CharacterMovementComponent = Cast<UPMCharacterMovementComponent>(AbilitySystemComponent->AbilityActorInfo->MovementComponent.Get());

		if (CharacterMovementComponent.IsValid())
		{
			TSharedPtr<FPhysCustomMovement_ConstantForce> movement = MakeShared<FPhysCustomMovement_ConstantForce>();

			movement->MovementName = CustomMovementName;
			movement->CharacterMovementComponent = CharacterMovementComponent;
			movement->MaxSpeed = MaxSpeed;
			movement->MaxAcceleration = MaxAcceleration;
			movement->MaxBrakingDeceleration = MaxBrakingDeceleration;
			movement->Force = Force;
			movement->OnCustomMovementEnd.AddDynamic(this, &ThisClass::OnPhysCustomMovementEnded);

			PhysCustomMovement = movement;
			bActivated = CharacterMovementComponent->StartPhysCustomMovement(PhysCustomMovement);
		}
	}
}
