// Copyright 2022 Andrey Bicalho.


#include "Characters/Abilities/AbilityTasks/GSAT_ApplyPhysCustomMovementBase.h"
#include "Characters/GSCharacterMovementComponent.h"
#include "GASShooter/GASShooter.h"

UGSAT_ApplyPhysCustomMovementBase::UGSAT_ApplyPhysCustomMovementBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bTickingTask = false;
	CustomMovementName = FName(NAME_None);
}

void UGSAT_ApplyPhysCustomMovementBase::OnPhysCustomMovementEnded()
{
	UE_LOG(LogTemp, Display, TEXT("%s: %s: Phys Custom Movement %s has finished on ability task %s"),
		ANSI_TO_TCHAR(__FUNCTION__),
		GET_ACTOR_LOCAL_ROLE_FSTRING(GetAvatarActor()),
		PhysCustomMovement.IsValid() ? *PhysCustomMovement->MovementName.ToString() : TEXT("Invalid"),
		*CustomMovementName.ToString());

	Finish();
}

void UGSAT_ApplyPhysCustomMovementBase::Activate()
{
	Super::Activate();

	if (!bActivated)
	{
		if (PhysCustomMovement.IsValid() && PhysCustomMovement->OnCustomMovementEnd.IsBound())
		{
			PhysCustomMovement->OnCustomMovementEnd.RemoveAll(this);
		}

		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnFailed.Broadcast();
		}

		EndTask();
	}
	else
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnSuccess.Broadcast();
		}
	}
}

void UGSAT_ApplyPhysCustomMovementBase::Finish()
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

void UGSAT_ApplyPhysCustomMovementBase::OnDestroy(bool AbilityIsEnding)
{
	if (PhysCustomMovement.IsValid() && PhysCustomMovement->OnCustomMovementEnd.IsBound())
	{
		PhysCustomMovement->OnCustomMovementEnd.RemoveAll(this);
	}

	if (PhysCustomMovement.IsValid() && PhysCustomMovement->IsActive())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: %s: Ability Task %s is being destroyed, but Phys Custom Movement %s is still valid and active. Movement mode will be set to Falling."),
			ANSI_TO_TCHAR(__FUNCTION__),
			GET_ACTOR_LOCAL_ROLE_FSTRING(GetAvatarActor()),
			*CustomMovementName.ToString(),
			*PhysCustomMovement->MovementName.ToString());

		if (CharacterMovementComponent)
		{
			CharacterMovementComponent->SetMovementMode(MOVE_Falling);
		}

		PhysCustomMovement.Reset();
	}

	Super::OnDestroy(AbilityIsEnding);
}
