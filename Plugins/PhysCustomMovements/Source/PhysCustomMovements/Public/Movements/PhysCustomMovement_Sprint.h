// Copyright 2022 Andrey Bicalho.

#pragma once

#include "Movements/PhysCustomMovement.h"
#include "PhysCustomMovement_Sprint.generated.h"

/**
* An example of a Sprint.
* 
*/
USTRUCT()
struct PHYSCUSTOMMOVEMENTS_API FPhysCustomMovement_Sprint : public FPhysCustomMovement
{
	GENERATED_USTRUCT_BODY()

	FPhysCustomMovement_Sprint();

	virtual ~FPhysCustomMovement_Sprint() {};

	virtual void UpdateMovement(const float deltaTime, const FVector& oldVelocity, FVector& outVelocity) override;
};
