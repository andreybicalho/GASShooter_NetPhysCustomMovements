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
																const FVector& inLaunchVelocity, bool bInXYOverride, bool bInZOverride)
{
	UGSAT_ApplyPhysCustomJump* jumpTask = NewAbilityTask<UGSAT_ApplyPhysCustomJump>(OwningAbility, TaskInstanceName);

	if (jumpTask)
	{
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
			// TODO: refactor to shared pointer?
			/*PhysJumpMovement = MakeShared<FPhysCustomMovement_Jump>();
			PhysJumpMovement->LaunchVelocity = LaunchVelocity;
			PhysJumpMovement->bXYOverride = bXYOverride;
			PhysJumpMovement->bZOverride = bZOverride;
			PhysJumpMovement->OnCustomMovementEnd.AddDynamic(this, &ThisClass::OnPhysJumpEnd);*/

			PhysJumpMovement.CharacterMovementComponent = CharacterMovementComponent;
			PhysJumpMovement.LaunchVelocity = LaunchVelocity;
			PhysJumpMovement.bXYOverride = bXYOverride;
			PhysJumpMovement.bZOverride = bZOverride;
			PhysJumpMovement.OnCustomMovementEnd.AddDynamic(this, &ThisClass::OnPhysJumpEnd);

			CharacterMovementComponent->StartPhysCustomMovement(PhysJumpMovement);
		}
	}
}

void UGSAT_ApplyPhysCustomJump::OnPhysJumpEnd()
{
	UE_LOG(LogTemp, Display, TEXT("%s: Jump Finished!"), ANSI_TO_TCHAR(__FUNCTION__));

	Finish();
}

void UGSAT_ApplyPhysCustomJump::Finish()
{
	if (PhysJumpMovement.OnCustomMovementEnd.IsBound())
	{
		PhysJumpMovement.OnCustomMovementEnd.RemoveAll(this);
	}

	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnFinish.Broadcast();
	}

	EndTask();
}

void UGSAT_ApplyPhysCustomJump::OnDestroy(bool AbilityIsEnding)
{
	if (PhysJumpMovement.OnCustomMovementEnd.IsBound())
	{
		PhysJumpMovement.OnCustomMovementEnd.RemoveAll(this);
	}

	if (PhysJumpMovement.IsActive())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: Movement is still active on character movement component. This can lead to a dangling pointer since task is ending but character movement component is still using the movement reference... trying to end the movement..."), ANSI_TO_TCHAR(__FUNCTION__));
		PhysJumpMovement.EndMovement(MOVE_Falling);
	}

	Super::OnDestroy(AbilityIsEnding);
}