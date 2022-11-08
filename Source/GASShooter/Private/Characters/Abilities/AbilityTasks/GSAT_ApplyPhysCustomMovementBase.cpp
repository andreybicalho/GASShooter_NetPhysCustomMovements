// Copyright 2022 Andrey Bicalho.


#include "Characters/Abilities/AbilityTasks/GSAT_ApplyPhysCustomMovementBase.h"

UGSAT_ApplyPhysCustomMovementBase::UGSAT_ApplyPhysCustomMovementBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bTickingTask = false;
	
	// TODO: should replicate?
	//bSimulatedTask = true;

	bIsFinished = false;
}

//void UGSAT_ApplyPhysCustomMovementBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
//{
//}
//
//void UGSAT_ApplyPhysCustomMovementBase::InitSimulatedTask(UGameplayTasksComponent& InGameplayTasksComponent)
//{
//	Super::InitSimulatedTask(InGameplayTasksComponent);
//
//	InitPhysCustomMovement();
//}