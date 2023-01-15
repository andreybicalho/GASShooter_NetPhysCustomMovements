// Copyright 2022 Andrey Bicalho.

#include "Movements/PhysCustomMovement_Sprint.h"
#include "GameFramework/CharacterMovementComponent.h"

FPhysCustomMovement_Sprint::FPhysCustomMovement_Sprint() : FPhysCustomMovement() {}

void FPhysCustomMovement_Sprint::UpdateMovement(const float deltaTime, const FVector& oldVelocity, FVector& outVelocity)
{
    SCOPED_NAMED_EVENT(FPhysCustomMovement_UpdateMovement_Sprint, FColor::Yellow);

    CurrentTime += deltaTime;

    outVelocity = CharacterMovementComponent->GetCurrentAcceleration() * GetMaxSpeed();
    outVelocity = outVelocity.GetClampedToMaxSize(GetMaxSpeed());
};
