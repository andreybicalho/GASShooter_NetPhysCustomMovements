// Copyright 2022 Andrey Bicalho.

#pragma once

#include "Movements/PhysCustomMovement.h"
#include "PhysCustomMovement_ConstantForce.generated.h"

/**
* An example of a Constant Force.
* 
*/
USTRUCT()
struct PHYSCUSTOMMOVEMENTS_API FPhysCustomMovement_ConstantForce : public FPhysCustomMovement
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FVector Force = FVector::ZeroVector;

	FPhysCustomMovement_ConstantForce();

	virtual ~FPhysCustomMovement_ConstantForce() {}

	virtual void UpdateMovement(const float deltaTime, const FVector& oldVelocity, FVector& outVelocity) override;
};
