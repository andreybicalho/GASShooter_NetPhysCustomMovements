// Copyright 2022 Andrey Bicalho.

#include "Movements/PhysCustomMovement_NonDeterministicMove.h"
#include "Components/PMCharacterMovementComponent.h"
#include "GameFramework/Character.h"

DEFINE_LOG_CATEGORY(LogPhysNonDeterministicMove);

FPhysCustomMovement_NonDeterministicMove::FPhysCustomMovement_NonDeterministicMove() : FPhysCustomMovement() {}

void FPhysCustomMovement_NonDeterministicMove::UpdateMovement(const float deltaTime, const FVector& oldVelocity, FVector& outVelocity)
{
    SCOPED_NAMED_EVENT(FPhysCustomMovement_UpdateMovement_NonDeterministicMove, FColor::Yellow);

    CurrentTime += deltaTime;

    ElapsedTime += deltaTime;

    if (TimeToWait > 0.f && ElapsedTime >= TimeToWait)
    {
        TimeToWait = -1.f;
        MovementDirectionSign *= -1.f;
        ElapsedTime = 0.f;
        OnReachedTime.Broadcast();
    }

    outVelocity = CharacterMovementComponent->GetCurrentAcceleration() * MovementDirectionSign * GetMaxSpeed();
    outVelocity = outVelocity.GetClampedToMaxSize(GetMaxSpeed());
};

UScriptStruct* FPhysCustomMovement_NonDeterministicMove::GetTypeStruct() const
{
    return FPhysCustomMovement_NonDeterministicMove::StaticStruct();
}
