// Copyright 2022 Andrey Bicalho.


#include "Characters/Abilities/AbilityTasks/GSAT_ApplyPhysCustomMovementBase.h"
#include "Characters/GSCharacterMovementComponent.h"
#include "GASShooter/GASShooter.h"

UGSAT_ApplyPhysCustomMovementBase::UGSAT_ApplyPhysCustomMovementBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bTickingTask = false;
}

void UGSAT_ApplyPhysCustomMovementBase::OnPhysCustomMovementEnded()
{
	UE_LOG(LogTemp, Display, TEXT("%s: %s: Phys Custom Movement Finished!"),
		ANSI_TO_TCHAR(__FUNCTION__),
		GET_ACTOR_LOCAL_ROLE_FSTRING(GetAvatarActor()));

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
		if (CharacterMovementComponent)
		{
			CharacterMovementComponent->StopPhysCustomMovement();
		}

		PhysCustomMovement.Reset();
	}

	Super::OnDestroy(AbilityIsEnding);
}
