// Copyright 2022 Andrey Bicalho.

#pragma once

#include "Movements/PhysCustomMovement.h"
#include "PhysCustomMovement_NonDeterministicMove.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPhysNonDeterministicMove, Log, All);

/**
* An example of a NonDeterministicMove.
* 
*/
USTRUCT()
struct PHYSCUSTOMMOVEMENTS_API FPhysCustomMovement_NonDeterministicMove : public FPhysCustomMovement
{
	GENERATED_USTRUCT_BODY()

	float TimeToWait = -1.f;

	float MovementDirectionSign = 1.f;

	float ElapsedTime = 0.f;

	FPhysCustomMovementDelegate OnReachedTime;

	FPhysCustomMovement_NonDeterministicMove();

	virtual ~FPhysCustomMovement_NonDeterministicMove() {};

	virtual void UpdateMovement(const float deltaTime, const FVector& oldVelocity, FVector& outVelocity) override;
};
