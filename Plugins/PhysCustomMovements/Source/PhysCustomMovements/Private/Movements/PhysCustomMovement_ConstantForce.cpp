// Copyright 2022 Andrey Bicalho.

#include "Movements/PhysCustomMovement_ConstantForce.h"
#include "GameFramework/CharacterMovementComponent.h"

FPhysCustomMovement_ConstantForce::FPhysCustomMovement_ConstantForce() : FPhysCustomMovement()
{
    Force = FVector::ZeroVector;
}

void FPhysCustomMovement_ConstantForce::UpdateMovement(const float deltaTime, const FVector& oldVelocity, FVector& outVelocity)
{
    SCOPED_NAMED_EVENT(FPhysCustomMovement_UpdateMovement_ConstantForce, FColor::Yellow);

    CurrentTime += deltaTime;

    const FVector velocity = Force * deltaTime;

    outVelocity += velocity;
    outVelocity = outVelocity.GetClampedToMaxSize(GetMaxSpeed());
}

//bool FPhysCustomMovement_ConstantForce::NetSerialize(FArchive& ar, UPackageMap* map, bool& bOutSuccess)
//{
//    return FPhysCustomMovement::NetSerialize(ar, map, bOutSuccess);
//}
//
//UScriptStruct* FPhysCustomMovement_ConstantForce::GetScriptStruct() const
//{
//    return FPhysCustomMovement_ConstantForce::StaticStruct();
//}
