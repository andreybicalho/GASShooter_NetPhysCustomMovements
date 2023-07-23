// Copyright 2022 Andrey Bicalho.

#pragma once

#include "Movements/PhysCustomMovement.h"
#include "PhysCustomMovement_Launch.generated.h"

/**
* An example of a Launch.
* 
*/
USTRUCT()
struct PHYSCUSTOMMOVEMENTS_API FPhysCustomMovement_Launch : public FPhysCustomMovement
{
	GENERATED_USTRUCT_BODY()

	FVector LaunchVelocity = FVector::ZeroVector;

	bool bXYOverride = false;

	bool bZOverride = false;

	FPhysCustomMovement_Launch();

	virtual ~FPhysCustomMovement_Launch() {}

	virtual void UpdateMovement(const float deltaTime, const FVector& oldVelocity, FVector& outVelocity) override;

	virtual void EndMovement() override;
};
