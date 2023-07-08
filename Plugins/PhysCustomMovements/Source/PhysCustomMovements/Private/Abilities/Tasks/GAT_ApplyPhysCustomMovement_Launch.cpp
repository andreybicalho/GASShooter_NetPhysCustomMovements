// Copyright 2022 Andrey Bicalho.


#include "Abilities/Tasks/GAT_ApplyPhysCustomMovement_Launch.h"
#include "Components/PMCharacterMovementComponent.h"
#include "Movements/PhysCustomMovement_Launch.h"
#include "AbilitySystemComponent.h"

UGAT_ApplyPhysCustomMovement_Launch::UGAT_ApplyPhysCustomMovement_Launch(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UGAT_ApplyPhysCustomMovement_Launch* UGAT_ApplyPhysCustomMovement_Launch::PhysLaunch(UGameplayAbility* OwningAbility, FName TaskInstanceName,
																const FVector& inLaunchVelocity, bool bInXYOverride, bool bInZOverride, 
																const float inMaxSpeed, const float inMaxAcceleration, const float MaxBrakingDeceleration)
{
	UGAT_ApplyPhysCustomMovement_Launch* abilityTask = NewAbilityTask<UGAT_ApplyPhysCustomMovement_Launch>(OwningAbility, TaskInstanceName);

	if (abilityTask)
	{
		abilityTask->CustomMovementName = TaskInstanceName;
		abilityTask->MaxSpeed = inMaxSpeed;
		abilityTask->MaxAcceleration = inMaxAcceleration;
		abilityTask->MaxBrakingDeceleration = MaxBrakingDeceleration;
		abilityTask->LaunchVelocity = inLaunchVelocity;
		abilityTask->bXYOverride = bInXYOverride;
		abilityTask->bZOverride = bInZOverride;
		
		abilityTask->InitAndApply();

		return abilityTask;
	}

	return nullptr;
}

void UGAT_ApplyPhysCustomMovement_Launch::InitAndApply()
{
	if (AbilitySystemComponent->AbilityActorInfo->MovementComponent.IsValid())
	{
		CharacterMovementComponent = Cast<UPMCharacterMovementComponent>(AbilitySystemComponent->AbilityActorInfo->MovementComponent.Get());
		
		if (CharacterMovementComponent.IsValid())
		{
			TSharedPtr<FPhysCustomMovement_Launch> movement = MakeShared<FPhysCustomMovement_Launch>();

			movement->MovementName = CustomMovementName;
			movement->MaxSpeed = MaxSpeed;
			movement->MaxAcceleration = MaxAcceleration;
			movement->MaxBrakingDeceleration = MaxBrakingDeceleration;
			movement->CharacterMovementComponent = CharacterMovementComponent;
			movement->LaunchVelocity = LaunchVelocity;
			movement->bXYOverride = bXYOverride;
			movement->bZOverride = bZOverride;
			movement->OnCustomMovementEnd.AddDynamic(this, &ThisClass::OnPhysCustomMovementEnded);

			Move(PhysCustomMovement, movement);
			bActivated = CharacterMovementComponent->StartPhysCustomMovement(PhysCustomMovement);
		}
	}
}
