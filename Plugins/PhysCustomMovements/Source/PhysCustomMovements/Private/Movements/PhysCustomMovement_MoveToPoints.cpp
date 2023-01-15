// Copyright 2022 Andrey Bicalho.

#include "Movements/PhysCustomMovement_MoveToPoints.h"
#include "GameFramework/CharacterMovementComponent.h"

FPhysCustomMovement_MoveToPoints::FPhysCustomMovement_MoveToPoints() : FPhysCustomMovement()
{
}

void FPhysCustomMovement_MoveToPoints::UpdateMovement(const float deltaTime, const FVector& oldVelocity, FVector& outVelocity)
{
    SCOPED_NAMED_EVENT(FPhysCustomMovement_UpdateMovement_MoveToPoints, FColor::Yellow);

    CurrentTime += deltaTime;

    if (!PathPoints.IsEmpty())
    {
        const FVector currentLocation = CharacterMovementComponent->GetActorFeetLocation();
        const FVector currentPoint = PathPoints.Top();

        const FVector direction = (currentPoint - currentLocation).GetSafeNormal();

        outVelocity = direction * GetMaxSpeed();

        outVelocity = outVelocity.GetClampedToMaxSize(GetMaxSpeed());

        const float distance = FVector::Distance(currentLocation, currentPoint);
        if (distance <= ConsumePointDistanceThreshold)
        {
            PathPoints.Pop();
        }
    }
    else
    {
        CharacterMovementComponent->SetMovementMode(MOVE_Falling);
    }
}
