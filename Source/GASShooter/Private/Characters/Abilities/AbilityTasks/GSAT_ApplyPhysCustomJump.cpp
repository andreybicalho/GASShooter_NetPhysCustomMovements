// Copyright 2022 Andrey Bicalho.


#include "Characters/Abilities/AbilityTasks/GSAT_ApplyPhysCustomJump.h"
#include "Characters/GSCharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GASShooter/GASShooter.h"

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

void UGSAT_ApplyPhysCustomJump::Activate()
{
	Super::Activate();
}

void UGSAT_ApplyPhysCustomJump::InitAndApply()
{
	if (AbilitySystemComponent->AbilityActorInfo->MovementComponent.IsValid())
	{
		CharacterMovementComponent = Cast<UGSCharacterMovementComponent>(AbilitySystemComponent->AbilityActorInfo->MovementComponent.Get());
		
		if (CharacterMovementComponent)
		{
			PhysCustomMovement = MakeShared<FPhysCustomMovement_Jump>();
			PhysCustomMovement->MovementName = CustomMovementName;
			PhysCustomMovement->MaxSpeed = MaxSpeed;
			PhysCustomMovement->CharacterMovementComponent = CharacterMovementComponent;
			PhysCustomMovement->LaunchVelocity = LaunchVelocity;
			PhysCustomMovement->bXYOverride = bXYOverride;
			PhysCustomMovement->bZOverride = bZOverride;
			PhysCustomMovement->MaxSpeed = MaxSpeed;
			PhysCustomMovement->OnCustomMovementEnd.AddDynamic(this, &ThisClass::OnPhysJumpEnd);

			CharacterMovementComponent->StartPhysCustomMovement(PhysCustomMovement);
		}
	}
}

void UGSAT_ApplyPhysCustomJump::OnPhysJumpEnd()
{
	UE_LOG(LogTemp, Display, TEXT("%s: %s: Jump Finished!"), 
		ANSI_TO_TCHAR(__FUNCTION__), 
		GET_ACTOR_LOCAL_ROLE_FSTRING(GetAvatarActor()));

	Finish();
}

void UGSAT_ApplyPhysCustomJump::Finish()
{
	if (PhysCustomMovement.IsValid() && PhysCustomMovement->OnCustomMovementEnd.IsBound())
	{
		PhysCustomMovement->OnCustomMovementEnd.RemoveAll(this);
	}

	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnFinish.Broadcast();
	}

	EndTask();
}

void UGSAT_ApplyPhysCustomJump::OnDestroy(bool AbilityIsEnding)
{
	if (PhysCustomMovement.IsValid() && PhysCustomMovement->OnCustomMovementEnd.IsBound())
	{
		PhysCustomMovement->OnCustomMovementEnd.RemoveAll(this);
	}

	if (PhysCustomMovement.IsValid() && PhysCustomMovement->IsActive())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: Movement is still active on character movement component. This can lead to a dangling pointer since task is ending but character movement component is still using the movement reference... trying to end the movement..."), ANSI_TO_TCHAR(__FUNCTION__));

		if (CharacterMovementComponent)
		{
			CharacterMovementComponent->StopPhysCustomMovement();
		}

		PhysCustomMovement.Reset();
	}

	Super::OnDestroy(AbilityIsEnding);
}