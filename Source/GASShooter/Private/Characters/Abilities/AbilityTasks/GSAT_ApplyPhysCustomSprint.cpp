// Copyright 2022 Andrey Bicalho.


#include "Characters/Abilities/AbilityTasks/GSAT_ApplyPhysCustomSprint.h"
#include "Characters/GSCharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GASShooter/GASShooter.h"

UGSAT_ApplyPhysCustomSprint::UGSAT_ApplyPhysCustomSprint(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UGSAT_ApplyPhysCustomSprint* UGSAT_ApplyPhysCustomSprint::PhysSprint(UGameplayAbility* OwningAbility, FName TaskInstanceName, float inMaxSpeed)
{
	UGSAT_ApplyPhysCustomSprint* abilityTask = NewAbilityTask<UGSAT_ApplyPhysCustomSprint>(OwningAbility, TaskInstanceName);

	if (abilityTask)
	{
		abilityTask->CustomMovementName = TaskInstanceName;
		abilityTask->MaxSpeed = inMaxSpeed;
		abilityTask->InitAndApply();

		return abilityTask;
	}

	return nullptr;
}

void UGSAT_ApplyPhysCustomSprint::Activate()
{
	Super::Activate();
}

void UGSAT_ApplyPhysCustomSprint::InitAndApply()
{
	if (AbilitySystemComponent->AbilityActorInfo->MovementComponent.IsValid())
	{
		CharacterMovementComponent = Cast<UGSCharacterMovementComponent>(AbilitySystemComponent->AbilityActorInfo->MovementComponent.Get());

		if (CharacterMovementComponent)
		{
			PhysCustomMovement = MakeShared<FPhysCustomMovement_Sprint>();
			PhysCustomMovement->MovementName = CustomMovementName;
			PhysCustomMovement->CharacterMovementComponent = CharacterMovementComponent;
			PhysCustomMovement->MaxSpeed = MaxSpeed;
			PhysCustomMovement->OnCustomMovementEnd.AddDynamic(this, &ThisClass::OnPhysSprintEnded);

			CharacterMovementComponent->StartPhysCustomMovement(PhysCustomMovement);
		}
	}
}

void UGSAT_ApplyPhysCustomSprint::OnPhysSprintEnded()
{
	UE_LOG(LogTemp, Display, TEXT("%s: %s: Sprint Finished!"),
		ANSI_TO_TCHAR(__FUNCTION__),
		GET_ACTOR_LOCAL_ROLE_FSTRING(GetAvatarActor()));

	Finish();
}

void UGSAT_ApplyPhysCustomSprint::Finish()
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

void UGSAT_ApplyPhysCustomSprint::OnDestroy(bool AbilityIsEnding)
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
