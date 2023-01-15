// Copyright 2022 Andrey Bicalho.

#include "Movements/PhysCustomMovement_Launch.h"
#include "GameFramework/CharacterMovementComponent.h"

FPhysCustomMovement_Launch::FPhysCustomMovement_Launch() : FPhysCustomMovement()
{
    LaunchVelocity = FVector::ZeroVector;
    bXYOverride = false;
    bZOverride = false;
}

void FPhysCustomMovement_Launch::UpdateMovement(const float deltaTime, const FVector& oldVelocity, FVector& outVelocity)
{
    SCOPED_NAMED_EVENT(FPhysCustomMovement_UpdateMovement_Launch, FColor::Yellow);

    CurrentTime += deltaTime;

    outVelocity = oldVelocity;

    if (!bXYOverride)
    {
        outVelocity.X += LaunchVelocity.X;
        outVelocity.Y += LaunchVelocity.Y;
    }
    else
    {
        outVelocity.X = LaunchVelocity.X;
        outVelocity.Y = LaunchVelocity.Y;
    }

    if (!bZOverride)
    {
        outVelocity.Z += LaunchVelocity.Z;
    }
    else
    {
        outVelocity.Z = LaunchVelocity.Z;
    }

    outVelocity = outVelocity.GetClampedToMaxSize(GetMaxSpeed());

    CharacterMovementComponent->SetMovementMode(MOVE_Falling);
};

void FPhysCustomMovement_Launch::EndMovement()
{
    LaunchVelocity = FVector::ZeroVector;

    FPhysCustomMovement::EndMovement();
}
