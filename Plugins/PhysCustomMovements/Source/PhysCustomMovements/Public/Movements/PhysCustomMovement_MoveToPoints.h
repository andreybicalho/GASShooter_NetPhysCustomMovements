// Copyright 2022 Andrey Bicalho.

#pragma once

#include "Movements/PhysCustomMovement.h"
#include "PhysCustomMovement_MoveToPoints.generated.h"

/**
* An example of moving to points.
*
*/
USTRUCT()
struct PHYSCUSTOMMOVEMENTS_API FPhysCustomMovement_MoveToPoints : public FPhysCustomMovement
{
	GENERATED_USTRUCT_BODY()

	TArray<FVector> PathPoints = {};

	float ConsumePointDistanceThreshold = 50.f;

	FPhysCustomMovement_MoveToPoints();

	virtual ~FPhysCustomMovement_MoveToPoints() {}

	virtual void UpdateMovement(const float deltaTime, const FVector& oldVelocity, FVector& outVelocity) override;
};
