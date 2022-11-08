// Copyright 2022 Andrey Bicalho.


#include "Characters/Abilities/AbilityTasks/GSAT_ApplyPhysCustomMovementBase.h"
#include "Net/UnrealNetwork.h"

UGSAT_ApplyPhysCustomMovementBase::UGSAT_ApplyPhysCustomMovementBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bTickingTask = false;
	bSimulatedTask = true;
}

void UGSAT_ApplyPhysCustomMovementBase::InitSimulatedTask(UGameplayTasksComponent& InGameplayTasksComponent)
{
	Super::InitSimulatedTask(InGameplayTasksComponent);
}

void UGSAT_ApplyPhysCustomMovementBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	DOREPLIFETIME(UGSAT_ApplyPhysCustomMovementBase, CustomMovementName);
}
