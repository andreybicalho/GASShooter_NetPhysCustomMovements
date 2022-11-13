// Copyright 2022 Andrey Bicalho.


#include "Characters/Abilities/AbilityTasks/GSAT_ApplyPhysCustomJump.h"
#include "Characters/GSCharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"

UGSAT_ApplyPhysCustomJump::UGSAT_ApplyPhysCustomJump(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UGSAT_ApplyPhysCustomJump* UGSAT_ApplyPhysCustomJump::PhysJump(UGameplayAbility* OwningAbility, FName TaskInstanceName,
																const FVector& inLaunchVelocity, bool bInXYOverride, bool bInZOverride, float inMaxSpeed)
{
	UGSAT_ApplyPhysCustomJump* jumpTask = NewAbilityTask<UGSAT_ApplyPhysCustomJump>(OwningAbility, TaskInstanceName);

	if (jumpTask)
	{
		jumpTask->CustomMovementName = TaskInstanceName;
		jumpTask->MaxSpeed = inMaxSpeed;
		jumpTask->LaunchVelocity = inLaunchVelocity;
		jumpTask->bXYOverride = bInXYOverride;
		jumpTask->bZOverride = bInZOverride;
		
		jumpTask->InitAndApply();

		return jumpTask;
	}

	return nullptr;
}

void UGSAT_ApplyPhysCustomJump::InitAndApply()
{
	if (AbilitySystemComponent->AbilityActorInfo->MovementComponent.IsValid())
	{
		CharacterMovementComponent = Cast<UGSCharacterMovementComponent>(AbilitySystemComponent->AbilityActorInfo->MovementComponent.Get());
		
		if (CharacterMovementComponent)
		{
			TSharedPtr<FPhysCustomMovement_Jump> movement = MakeShared<FPhysCustomMovement_Jump>();

			movement->MovementName = CustomMovementName;
			movement->MaxSpeed = MaxSpeed;
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

