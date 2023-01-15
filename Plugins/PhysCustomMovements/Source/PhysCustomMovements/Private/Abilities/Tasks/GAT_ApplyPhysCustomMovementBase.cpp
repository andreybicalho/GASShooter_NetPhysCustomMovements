// Copyright 2022 Andrey Bicalho.


#include "Abilities/Tasks/GAT_ApplyPhysCustomMovementBase.h"
#include "Components/PMCharacterMovementComponent.h"

UGAT_ApplyPhysCustomMovementBase::UGAT_ApplyPhysCustomMovementBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bTickingTask = false;
	CustomMovementName = FName(NAME_None);
}

void UGAT_ApplyPhysCustomMovementBase::OnPhysCustomMovementEnded()
{
	UE_LOG(LogPhysCustomMovement, Display, TEXT("%s: %s: Phys Custom Movement %s has finished on ability task %s"),
		ANSI_TO_TCHAR(__FUNCTION__),
		GET_ACTOR_LOCAL_ROLE_FSTRING(GetAvatarActor()),
		PhysCustomMovement.IsValid() ? *PhysCustomMovement->MovementName.ToString() : TEXT("Invalid"),
		*CustomMovementName.ToString());

	Finish();
}

void UGAT_ApplyPhysCustomMovementBase::Activate()
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

void UGAT_ApplyPhysCustomMovementBase::Finish()
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

void UGAT_ApplyPhysCustomMovementBase::OnDestroy(bool AbilityIsEnding)
{
	if (PhysCustomMovement.IsValid() && PhysCustomMovement->OnCustomMovementEnd.IsBound())
	{
		PhysCustomMovement->OnCustomMovementEnd.RemoveAll(this);
	}

	if (PhysCustomMovement.IsValid() && PhysCustomMovement->IsActive())
	{
		UE_LOG(LogPhysCustomMovement, Warning, TEXT("%s: %s: Ability Task %s is being destroyed, but Phys Custom Movement %s is still valid and active. Movement mode will be set to Falling."),
			ANSI_TO_TCHAR(__FUNCTION__),
			GET_ACTOR_LOCAL_ROLE_FSTRING(GetAvatarActor()),
			*CustomMovementName.ToString(),
			*PhysCustomMovement->MovementName.ToString());

		if (CharacterMovementComponent.IsValid())
		{
			CharacterMovementComponent->SetMovementMode(MOVE_Falling);
		}

		PhysCustomMovement.Reset();
	}

	Super::OnDestroy(AbilityIsEnding);
}
