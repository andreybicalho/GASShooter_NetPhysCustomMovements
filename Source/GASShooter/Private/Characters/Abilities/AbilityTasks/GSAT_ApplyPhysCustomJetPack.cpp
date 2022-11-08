// Copyright 2022 Andrey Bicalho.


#include "Characters/Abilities/AbilityTasks/GSAT_ApplyPhysCustomJetPack.h"
#include "Characters/GSCharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GASShooter/GASShooter.h"

UGSAT_ApplyPhysCustomJetPack::UGSAT_ApplyPhysCustomJetPack(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UGSAT_ApplyPhysCustomJetPack* UGSAT_ApplyPhysCustomJetPack::PhysJetPack(UGameplayAbility* OwningAbility, FName TaskInstanceName, const FVector& inJetPackAcceleration)
{
	UGSAT_ApplyPhysCustomJetPack* abilityTask = NewAbilityTask<UGSAT_ApplyPhysCustomJetPack>(OwningAbility, TaskInstanceName);

	if (abilityTask)
	{
		abilityTask->CustomMovementName = TaskInstanceName;
		abilityTask->JetPackAcceleration = inJetPackAcceleration;
		abilityTask->InitAndApply();

		return abilityTask;
	}

	return nullptr;
}

void UGSAT_ApplyPhysCustomJetPack::Activate()
{
	Super::Activate();
}

void UGSAT_ApplyPhysCustomJetPack::InitAndApply()
{
	if (AbilitySystemComponent->AbilityActorInfo->MovementComponent.IsValid())
	{
		CharacterMovementComponent = Cast<UGSCharacterMovementComponent>(AbilitySystemComponent->AbilityActorInfo->MovementComponent.Get());

		if (CharacterMovementComponent)
		{
			// TODO: refactor to shared pointer?
			PhysJetPackMovement.MovementName = CustomMovementName;
			PhysJetPackMovement.CharacterMovementComponent = CharacterMovementComponent;
			PhysJetPackMovement.MaxSpeed = 2022.f;
			PhysJetPackMovement.BaseAcceleration = JetPackAcceleration;
			PhysJetPackMovement.OnCustomMovementEnd.AddDynamic(this, &ThisClass::OnPhysJetPackEnded);

			CharacterMovementComponent->StartPhysCustomMovement(PhysJetPackMovement);
		}
	}
}

void UGSAT_ApplyPhysCustomJetPack::OnPhysJetPackEnded()
{
	UE_LOG(LogTemp, Display, TEXT("%s: %s: JetPack Finished!"),
		ANSI_TO_TCHAR(__FUNCTION__),
		GET_ACTOR_LOCAL_ROLE_FSTRING(GetAvatarActor()));

	Finish();
}

void UGSAT_ApplyPhysCustomJetPack::Finish()
{
	if (PhysJetPackMovement.OnCustomMovementEnd.IsBound())
	{
		PhysJetPackMovement.OnCustomMovementEnd.RemoveAll(this);
	}

	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnFinish.Broadcast();
	}

	EndTask();
}

void UGSAT_ApplyPhysCustomJetPack::OnDestroy(bool AbilityIsEnding)
{
	if (PhysJetPackMovement.OnCustomMovementEnd.IsBound())
	{
		PhysJetPackMovement.OnCustomMovementEnd.RemoveAll(this);
	}

	if (PhysJetPackMovement.IsActive())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: Movement is still active on character movement component. This can lead to a dangling pointer since task is ending but character movement component is still using the movement reference... trying to end the movement..."), ANSI_TO_TCHAR(__FUNCTION__));

		if (CharacterMovementComponent)
		{
			CharacterMovementComponent->StopPhysCustomMovement();
		}

		PhysJetPackMovement.EndMovement();
	}

	Super::OnDestroy(AbilityIsEnding);
}